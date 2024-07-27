#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include "CPSHandler.h"
#include "SensorManager.h"
#include "jsonutil.h"
#include <queue>
#include <mutex>

/////////////////////////////////////////////////////////////////////////////////////
//void ComLoop(Handler * handler, int freq, int float_num)
//{
//	BufferedAsyncSerial* mycom = nullptr;
//	while (true)
//	{
//		const char* com_name = "COM6";
//		unsigned int baud_rate = 115200;
//		try
//		{
//			if (!mycom)
//			{
//				mycom = new BufferedAsyncSerial();
//			}
//			if (!mycom->isOpen())
//			{
//				mycom->open(com_name, baud_rate);
//			}
//		}
//		catch (const std::exception& e)
//		{
//			printf("open com: %s failed, %s!\n", com_name, e.what());
//			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//			continue;
//		}
//		try
//		{
//			if (mycom->errorStatus())
//			{
//				try
//				{
//					mycom->close();
//				}
//				catch (const std::exception&)
//				{}
//				delete mycom;
//				mycom = nullptr;
//				continue;
//			}
//		}
//		catch (const std::exception& e)
//		{
//			printf("com %s error, %s!\n", com_name, e.what());
//			std::this_thread::sleep_for(std::chrono::milliseconds(100));
//			continue;
//		}
//
//		std::string line = mycom->readStringUntil("\n");
//		std::vector<std::string> nums;
//		boost::split(nums, line, boost::is_any_of(","));
//		if (nums.size() == float_num)
//		{
//			try
//			{
//				ST_SensorData data;
//				data.sensor_num = nums.size();
//				for (size_t i = 0; i < nums.size() && i < MAX_SENSOR_DATA_NUM; i++)
//				{
//					data.data[i] = boost::lexical_cast<float>(nums[i]);
//				}
//				handler->SendSensorData(-1, &data);
//				std::cout << line << std::endl;
//			}
//			catch (const std::exception& e)
//			{
//				printf("parse %s failed!\n", line);
//			}
//		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / freq));
//	}
//}

bool g_exit = false;

void GetTimeStr(char *dt, size_t len)
{
	time_t t = time(NULL);
	struct tm* gtimeinfo = localtime(&t);
	strftime(dt, len, "%Y%m%d%H%M%S", gtimeinfo);
}

void SerializeFunc(SensorManager* mng, SensorCfg* sc)
{
	FILE* fp = NULL; 

	std::queue<ST_SensorData> queue_copy;
	while (!g_exit)
	{
		if (!sc->is_serialize_data)
		{
			if (fp)
			{
				fclose(fp);
				fp = NULL;
				printf("结束采集数据到文件。\n");
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		if(!fp)
		{
			char filename[256] = { 0 };
			char dt[64] = { 0 };
			GetTimeStr(dt, sizeof(dt));
			snprintf(filename, sizeof(filename), "sensor_data_%s.txt", dt);
			fp = fopen(filename, "wb");
			printf("开始采集数据到文件%s\n", filename);
			// 抛弃之前的数据
			std::vector<std::string> vec_data;
			mng->RetrieveAllData(vec_data);
		}
		// 等待一段时间
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//auto t = std::chrono::steady_clock::now();

		std::vector<std::string> vec_data;
		mng->RetrieveAllData(vec_data);
		printf("正在写入%d条数据！\n", vec_data.size());
		for (size_t i = 0; i < vec_data.size(); i++)
		{
			if (fp)
			{
				fprintf(fp, "%s\n", vec_data[i].c_str());
			}
			/*ST_SensorData data = { 0 };
			if(!mng->DecodeStr(vec_data[i], &data))
			{
				printf("解析%s失败！\n", vec_data[i].c_str());
				continue; 
			}
			if (fp)
			{
				for (unsigned int i = 0; i < data.sensor_num; i++)
				{
					fprintf(fp, "%.2f", data.data[i]);
					if (i != data.sensor_num - 1)
					{
						fprintf(fp, ",");
					}
				}
				fprintf(fp, "\n");
			}*/
		}
		printf("写入%d条数据完成！\n", vec_data.size());
	}
	if (fp)
	{
		fclose(fp);
	}
}

void BroadCastFunc(Handler* handler, SensorManager * mng, SensorCfg * sc)
{
	while (!g_exit)
	{
		if (mng->IsSensorOK())
		{
			ST_SensorData data = mng->GetCurrentSensorData();
			SYSTEMTIME sys;
			char send_t[64] = { 0 }, arrive_t[64] = { 0 };
			snprintf(send_t, 64, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);

			GetLocalTime(&sys);
			strcpy(data.t1, send_t);
			strcpy(data.t2, arrive_t);
			handler->SendSensorData(-1, &data);

			// 根据配置需要打印传感数据
			if (sc->is_print_sensor_data && data.sensor_num > 0)
			{
				for (unsigned int i = 0; i < data.sensor_num; i++)
				{
					printf("%.2f", data.data[i]);
					if (i != data.sensor_num)
					{
						printf(",");
					}
				}
				printf("\n");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / sc->broadcast_freq));
	}
}

int main()
{
	const char* cfg_file = "sensor_server_cfg.json";

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

	SensorCfg sc = { 0 };
	sc.broadcast_freq = doc["sensor"]["broadcast_freq"].GetUint();
	sc.baud_rate = doc["sensor"]["baud_rate"].GetUint();
	sc.sensor_num = doc["sensor"]["sensor_num"].GetUint();
	strncpy(sc.com_name, doc["sensor"]["com_name"].GetString(), sizeof(sc.com_name) - 1);
	strncpy(sc.delim, doc["sensor"]["delim"].GetString(), sizeof(sc.delim) - 1);
	sc.is_broadcast_data = doc["sensor"]["is_broadcast_data"].GetBool();
	sc.is_serialize_data = doc["sensor"]["is_serialize_data"].GetBool();
	sc.is_print_sensor_data = doc["sensor"]["is_print_sensor_data"].GetBool();
	sc.is_print_raw_string = doc["sensor"]["is_print_raw_string"].GetBool();

	// CPS API
	CCPSAPI* cps_api = CCPSAPI::CreateAPI();
	Handler handler(cps_api);
	if (sc.is_broadcast_data)
	{
		if (cps_api->Init(E_CPS_TYPE_DEVICE, SENSOR_SERVER_DEV_ID, 0,
			bus_ip, bus_port, log_ip, log_port, &handler) != 0)
		{
			printf("API init failed.\n");
			return EXIT_FAILURE;
		}
	}
	SensorManager mng(sc);
	if (!mng.LaunchService())
	{
		return EXIT_FAILURE;
	}

	std::thread t_broadcast;
	if (sc.is_broadcast_data)
	{
		t_broadcast = std::move(std::thread(BroadCastFunc, &handler, &mng, &sc));
	}
	std::thread t_serialize(SerializeFunc, &mng, &sc);
	
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
			snprintf(filename, sizeof(filename), "current_sensor_data.txt");
			ST_SensorData data = mng.GetCurrentSensorData();
			if (data.sensor_num > 0)
			{
				FILE* fp = fopen(filename, "a+");
				if (fp)
				{
					char dt[64] = { 0 };
					GetTimeStr(dt, sizeof(dt));
					fprintf(fp, "%s", dt);
					for (unsigned int i = 0; i < data.sensor_num; i++)
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

	t_serialize.join();
	mng.StopService();

	if (cps_api)
		cps_api->Release();
	return 0;
}
