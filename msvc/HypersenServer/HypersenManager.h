#pragma once
#include "BufferedAsyncSerial.h"
#include "CPSHypersenServerDef.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include "CPSAPI/CPSAPI.h"


struct HypersenCfg
{
	unsigned int broadcast_freq; // ÍÆËÍÆµÂÊ
	unsigned int baud_rate;
	char		 com_name[16];
	bool		 is_broadcast_data;
	bool		 is_serialize_data;
	bool		 is_print_sensor_data;
};


class HypersenManager
{
public:
	HypersenManager(CCPSAPI* cps_api, const HypersenCfg & cfg);

	~HypersenManager();

	bool LaunchService();
	void StopService();

	bool IsSensorOK();
	bool IsReading();

	const ST_HypersenSensorData & GetCurrentSensorData();
	const ST_HypersenSensorInfo& GetSensorInfo();
	const ST_HypersenSensorStatus& GetSensorStatus();

	void ReadSensorDevID();
	void ReadVersion();
	void StartRead();
	void StopRead();
	void ResetZero();

protected:
	void ReadThreadFunc();

	bool CheckComConnection();
	void DestroyComConnection();
protected:
	HypersenCfg m_sc;
	BufferedAsyncSerial* m_com = nullptr;
	std::atomic_bool m_is_sensor_ok = false;

	ST_HypersenSensorStatus m_sensor_status = { 0 };

	std::mutex m_sensor_lock;
	ST_HypersenSensorData m_cur_sensor_data = { 0 };

	ST_HypersenSensorInfo m_sensor_info = { 0 };

	bool m_exit_flag = false;
	std::thread m_thread_handle;

	CCPSAPI* m_cps_api;
};

