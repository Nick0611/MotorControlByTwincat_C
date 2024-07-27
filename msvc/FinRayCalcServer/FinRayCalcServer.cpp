#include "CPSAPI/CPSAPI.h"
#include "CPSAPI/CPSDef.h"
#include "CPSSensorServerDef.h"
#include "CPSFinRayCalcServerDef.h"
#include <stdio.h>
#include <queue>
#include <mutex>
#include <thread>
#include <Windows.h>
#include "ppcm.h"
#include <atomic>
#include <string.h>
#include "MyHandler.h"
#include "jsonutil.h"

static const int RequestLimit = 300;//定义队列里最多存储的请求数
// 线程退出标志
std::atomic_bool g_exit_flag = false;

//定义存储计算请求的全局结构体，生产者和消费者操作该结构
struct RequestRepository
{
	std::queue<ST_SensorData> SnsCalcReq;// 感知计算请求队列
	std::queue<uint32_t> sns_from_id;//感知计算请求来自...
	std::mutex sns_mtx; //感知互斥量，保护请求存储区
	std::mutex calc_mtx;//互斥量，保护计算线程对缓冲区访问的不冲突
	std::condition_variable repo_not_full_sns;//条件变量，指示感知请求缓冲区不为满
	std::condition_variable repo_not_empty_sns;//条件变量，指示感知请求缓冲区不为空
}RequestRepo;

class Handler : public CCPSEventHandler
{
public:
	Handler(CCPSAPI* api, RequestRepository* ir) :m_api(api), mybuffer(ir)
	{
		sns_statusinit(&cur_sns_status); //机构状态初始化
	}
	// Connect event
	virtual void OnConnected()
	{
		m_api->RegisterDevice();
		printf("OnConnected\n");
	}
	// Disconnect event
	virtual void OnDisconnected()
	{
		printf("OnDisconnected\n");
	}
	// Message event //添加自己东西
	virtual void OnMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
	{
		switch (msg_type)
		{
		case MSG_CMD_INIT:  // 计算服务初始化
			this->OnInit(from_id, (ST_CMDInit*)data);
			break;
		case MSG_CMD_STOP:  // 计算服务停止
			this->OnStop(from_id, (ST_CMDStop*)data);
			break;
		//case RRI_MSG_REQ_CALC:  // 请求感知计算
		//	this->OnReqCalcSns(from_id, (ST_RRI_REQ_CALC*)data);
		case MSG_SENSOR_DATA:  // 请求感知计算
		this->OnReqCalcSns(from_id, (ST_SensorData*)data);
			break;
		default:
			break;
		}
	}

	void OnInit(uint32_t from_id, ST_CMDInit* init)
	{
		ST_CMDInitRsp rsp = { 0 };
		rsp.req_no = init->req_no;

		m_api->SendDeviceMsg(from_id, MSG_CMD_INIT_RSP, (const char*)(&rsp), sizeof(ST_CMDInitRsp));
	}

	void OnStop(uint32_t from_id, ST_CMDStop* req)
	{
		printf("收到停止请求");
		ST_CMDStopRsp Rsp = { 0 };
		Rsp.req_no = req->req_no;
		Rsp.rsp.error_code = 0;
		strcpy_s(Rsp.rsp.error_msg, " Stop Success");
		m_api->SendDeviceMsg(from_id, MSG_CMD_STOP_RSP, (const char*)&Rsp, sizeof(ST_CMDStopRsp));
	}

	// 收到计算请求后，立即返回应答，并将计算请求存入自定义的缓存区
	void OnReqCalcSns(uint32_t from_id, ST_SensorData* req)
	{
		char receive_t[64] = { 0 }, send_t[64] = { 0 };
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		snprintf(receive_t, 64, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		std::cout << "线程 (" << std::this_thread::get_id();
		printf(") 收到感知计算请求from: %d, 应变片数量: %d, 电压1: %f, 电压2: %f, 电压3: %f, 电压4: %f\n",
			from_id, req->sensor_num, req->data[0], req->data[1], req->data[2], req->data[3]);

		////////////////调用感知计算函数////////////////////////
		sensingresult result;
		double sensor[4] = { req->data[0],req->data[1],req->data[2],req->data[3] };
		result = SensingBlock(&cur_sns_status, sensor);
		///////////////////////////////////////////////////

		//构造计算结果
		ST_RRI_Calc_Result rsp = { 0 };


		rsp.req_id = req->sensor_num;
		rsp.info.error_code = result.error_code;

		rsp.contact_force = result.fc * 2.5e5;

		rsp.contact_position[0] = result.rC(0);
		rsp.contact_position[1] = result.rC(1);
		rsp.contact_position[2] = result.rC(2);

		rsp.contact_direction[0] = result.xC(0);
		rsp.contact_direction[1] = result.xC(1);
		rsp.contact_direction[2] = result.xC(2);

		//cout << "\n肋板位置" << result.rib << endl;

		for (int i = 0; i < RIB_NUM * 2; i++)
		{
			rsp.ribx[i] = result.rib(0, i);
			rsp.riby[i] = result.rib(1, i);
		}

		for (int i = 0; i < RRI_SEG_NUM; i++)
		{
			rsp.q[i] = result.q(i);
			rsp.rxix[i] = result.rxi(0, i);
			rsp.rxiy[i] = result.rxi(1, i);
		}
		GetLocalTime(&sys);
		snprintf(send_t, 64, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		strcpy(rsp.t1, req->t1);
		strcpy(rsp.t2, req->t2);
		strcpy(rsp.t3, req->t3);
		strcpy(rsp.t4, receive_t);
		strcpy(rsp.t5, send_t);
		m_api->SendDeviceMsg(from_id, RRI_MSG_CALC_RESULT, (const char*)&rsp, sizeof(ST_RRI_Calc_Result)); //发送计算结果
		printf("发送感知计算结果，id: %d\n", rsp.req_id);
	
	}
private:
	CCPSAPI* m_api;
	RequestRepository* mybuffer;
	statusrecord cur_sns_status;
};




int main()
{
	const char* cfg_file = "finray_calc_server_cfg.json";

	// parse json configure file
	Document doc;
	if (!read_from_file(doc, cfg_file))
	{
		fprintf(stderr, "Parsing file %s failed.\n", cfg_file);
		fflush(stderr);
		return EXIT_FAILURE;
	}

	const char* log_ip = doc["log"]["ip"].GetString();
	unsigned int log_port = doc["log"]["port"].GetUint();

	const char* bus_ip = doc["bus"]["ip"].GetString();
	unsigned int bus_port = doc["bus"]["port"].GetUint();


	CCPSAPI* api = CCPSAPI::CreateAPI();
	Handler handler(api, &RequestRepo);
	if (api->Init(E_CPS_TYPE_DEVICE, FINRAY_CALC_SERVER_DEV_ID, 0,
		bus_ip, bus_port, log_ip, log_port, &handler) != 0)//"139.196.41.146", 7500
	{
		printf("API init failed.\n");
		return -1;
	}

	while (true)
	{
		int q = 0;
		std::cin >> q;

		if (q == 0)
		{
			break;
		}
	}
	//退出线程
	g_exit_flag = true;
	// exit
	api->Release();
	return 0;
}

