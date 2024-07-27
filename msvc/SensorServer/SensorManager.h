#pragma once
#include "BufferedAsyncSerial.h"
#include "CPSSensorServerDef.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>

struct SensorCfg
{
	unsigned int broadcast_freq; // ÍÆËÍÆµÂÊ
	unsigned int baud_rate;
	char		 com_name[16];
	unsigned int sensor_num;
	char		 delim[16];
	bool		 is_broadcast_data;
	bool		 is_serialize_data;
	bool		 is_print_sensor_data;
	bool		 is_print_raw_string;
};

class SensorManager
{
public:
	SensorManager(const SensorCfg &sc);
	~SensorManager();

	bool LaunchService();
	void StopService();

	bool IsSensorOK();

	ST_SensorData GetCurrentSensorData();
	bool DecodeStr(const std::string& line, ST_SensorData* data);

	void RetrieveAllData(std::vector<std::string>& data);
protected:
	void ReadThreadFunc();

	bool CheckComConnection();
	void DestroyComConnection();
protected:
	SensorCfg m_sc;
	BufferedAsyncSerial* m_com = nullptr;
	std::atomic_bool m_is_sensor_ok = false;

	ST_SensorData m_cur_sensor_data = { 0 };

	std::mutex m_sensor_lock;
	std::vector<std::string> m_sensor_data_strs;

	bool m_exit_flag = false;
	std::thread m_thread_handle;
};

