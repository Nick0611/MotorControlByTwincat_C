#include <stdio.h>
#include <iostream>
#include <thread>
#include "CPSHandler.h"
#include "HypersenManager.h"
#include "jsonutil.h"
#include <queue>
#include <mutex>


bool g_exit = false;

void GetTimeStr(char* dt, size_t len)
{
	time_t t = time(NULL);
	struct tm* gtimeinfo = localtime(&t);
	strftime(dt, len, "%Y%m%d%H%M%S", gtimeinfo);
}

//void SerializeFunc(HypersenManager* mng, HypersenCfg* sc)
//{
//	FILE* fp = NULL;
//
//	std::queue<ST_HypersenSensorData> queue_copy;
//	while (!g_exit)
//	{
		//if (!sc->is_serialize_data)
		//{
		//	if (fp)
		//	{
		//		fclose(fp);
		//		fp = NULL;
		//		printf("结束采集数据到文件。\n");
		//	}
		//	std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//	continue;
		//}
		//if (!fp)
		//{
		//	char filename[256] = { 0 };
		//	char dt[64] = { 0 };
		//	GetTimeStr(dt, sizeof(dt));
		//	snprintf(filename, sizeof(filename), "sensor_data_%s.txt", dt);
		//	fp = fopen(filename, "wb");
		//	printf("开始采集数据到文件%s\n", filename);
		//	// 抛弃之前的数据
		//	std::vector<std::string> vec_data;
		//	mng->RetrieveAllData(vec_data);
		//}
		// 等待一段时间
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));

		//std::vector<std::string> vec_data;
		//mng->RetrieveAllData(vec_data);
		//printf("正在写入%d条数据！\n", vec_data.size());
		//for (size_t i = 0; i < vec_data.size(); i++)
		//{
		//	if (fp)
		//	{
		//		fprintf(fp, "%s\n", vec_data[i].c_str());
		//	}
		//}
		//printf("写入%d条数据完成！\n", vec_data.size());
	//}
	//if (fp)
	//{
	//	fclose(fp);
	//}
//}

void BroadCastFunc(Handler* handler, HypersenManager* mng, HypersenCfg* sc)
{
	while (!g_exit)
	{
		if (mng->IsSensorOK() && mng->IsReading())
		{
			ST_HypersenSensorData data = mng->GetCurrentSensorData();
			handler->SendSensorData(-1, &data);
			// 根据配置需要打印传感数据
			if (sc->is_print_sensor_data && data.code == 0)
			{
				for (unsigned int i = 0; i < HYPERSEN_SENSOR_DOF; i++)
				{
					printf("%.2f", data.data[i]);
					if (i != HYPERSEN_SENSOR_DOF - 1)
					{
						printf(",");
					}
				}
				printf("\n");
			}
		}
		//else 
		//{
		//	if (!mng->IsSensorOK())
		//		printf("传感器状态错误!\n");
		//	if (!mng->IsReading())
		//		printf("未读取数据！\n");
		//}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / sc->broadcast_freq));
	}
}

int main()
{
	const char* cfg_file = "hypersen_server_cfg.json";
	
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

	HypersenCfg sc = { 0 };
	sc.broadcast_freq = doc["sensor"]["broadcast_freq"].GetUint();
	sc.baud_rate = doc["sensor"]["baud_rate"].GetUint();
	strncpy(sc.com_name, doc["sensor"]["com_name"].GetString(), sizeof(sc.com_name) - 1);
	sc.is_broadcast_data = doc["sensor"]["is_broadcast_data"].GetBool();
	sc.is_serialize_data = doc["sensor"]["is_serialize_data"].GetBool();
	sc.is_print_sensor_data = doc["sensor"]["is_print_sensor_data"].GetBool();

	// CPS API
	CCPSAPI* cps_api = CCPSAPI::CreateAPI();
	HypersenManager mng(cps_api, sc);
	Handler handler(cps_api, &mng);
	if (sc.is_broadcast_data)
	{
		if (cps_api->Init(E_CPS_TYPE_DEVICE, HYPERSEN_SENSOR_DEV_ID, 0,
			bus_ip, bus_port, log_ip, log_port, &handler) != 0)
		{
			printf("API init failed.\n");
			return EXIT_FAILURE;
		}
	}
	
	if (!mng.LaunchService())
	{
		return EXIT_FAILURE;
	}

	std::thread t_broadcast;
	if (sc.is_broadcast_data)
	{
		t_broadcast = std::move(std::thread(BroadCastFunc, &handler, &mng, &sc));
	}
	//std::thread t_serialize(SerializeFunc, &mng, &sc);

	printf("enter q to exit: \n");
	while (true)
	{
		char q = 0;
		std::cin >> q;

		if (q == 'q')
		{
			break;
		}
		else if (q == 'c')
		{
			// save current sensor data to file
			char filename[256] = { 0 };
			snprintf(filename, sizeof(filename), "current_hypersen_sensor_data.txt");

			ST_HypersenSensorData data = mng.GetCurrentSensorData();
			if (data.code == 0)
			{
				FILE* fp = fopen(filename, "a+");
				if (fp)
				{
					char dt[64] = { 0 };
					GetTimeStr(dt, sizeof(dt));
					fprintf(fp, "%s", dt);
					for (unsigned int i = 0; i < HYPERSEN_SENSOR_DOF; i++)
					{
						fprintf(fp, ", %.2f", data.data[i]);
					}
					fprintf(fp, "\n");
					fclose(fp);
					printf("存储当前传感数据到文件%s\n", filename);
				}
			}
			else
			{
				printf("没有采集到数据，存储当前传感数据请求失败！\n");
			}
		}
		else if (q == 's')
		{
			sc.is_serialize_data = true;
		}
		else if (q == 't')
		{
			sc.is_serialize_data = false;
		}
	}
	// exit
	g_exit = true;
	if (sc.is_broadcast_data)
		t_broadcast.join();

	//t_serialize.join();
	mng.StopService();

	if (cps_api)
		cps_api->Release();
	return 0;
}