#pragma once
#include "UIBaseWindow.h"
#include "CPSSensorServerDef.h"
#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include "UIPlotDef.h"
#include <string.h>
#include <windows.h>
#include <stdio.h>

class UISensorMonitor :
    public UIBaseWindow
{
public:
	UISensorMonitor(UIGLWindow* main_win, const char* title);
	virtual ~UISensorMonitor();

	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_TOOL; }
	virtual const char* GetShowShortCut() { return "Ctrl+2"; }

	virtual void OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len);

	void GetCurrentSensorData(ST_SensorData& data);
protected:
	void OnSensorData(uint32_t from_id, const char* data, uint32_t msg_len);
	void ResetCache();
	void RetrieveAllData(std::vector<ST_SensorData>& vec_data);
	static void SerializeFunc(void * arg);
protected:
	std::mutex m_cur_data_lock;
	ST_SensorData m_cur_data = { 0 };
	float initial_value[MAX_SENSOR_DATA_NUM] = { 0 };//应变片初值（用于置零）
	std::atomic_bool reset_initial_value = false;

	std::mutex m_vec_data_lock;
	std::vector<ST_SensorData> m_vec_data;

	bool m_exiting = false;
	std::thread m_serialize_thread;
	std::atomic_bool m_start_serialize = false;
	std::atomic_bool m_start_singlepoint_sampling = false;

	// 数据曲线，最多3条
	const unsigned int m_max_line_plot_num = 4;
	
	std::mutex m_vec_hist_data_lock;
	std::vector<ScrollingBuffer> m_vec_hist_data;

	char filename[128] = {0}; // 单点采集数据文件名
};

