#pragma once
#include "UIBaseWindow.h"
#include "CPSHypersenServerDef.h"
#include "UIPlotDef.h"
#include <mutex>
#include <vector>
#include "UIUtils.h"

class UIHypersenMonitor :  public UIBaseWindow
{
public:
	UIHypersenMonitor(UIGLWindow* main_win, const char* title);
	~UIHypersenMonitor();

	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_TOOL; }
	virtual const char* GetShowShortCut() { return "Ctrl+4"; }

	virtual void OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len);
protected:
	void OnSensorStatus(uint32_t from_id, const char* data, uint32_t msg_len);
	void OnSensorInfo(uint32_t from_id, const char* data, uint32_t msg_len);
	void OnSensorData(uint32_t from_id, const char* data, uint32_t msg_len);
protected:
	void StartMeasure();
	void StopMeasure();
	void ResetZero();
	void GetSensorInfo();
	void GetSensorStatus();
protected:
	std::mutex m_cur_data_lock;
	ST_HypersenSensorData m_cur_data = { 0 };

	std::mutex m_sensor_status_lock;
	ST_HypersenSensorStatus m_sensor_status = { 0 };

	std::mutex m_sensor_info_lock;
	ST_HypersenSensorInfo m_sensor_info = { 0 };

	std::mutex m_vec_hist_data_lock;
	std::vector<ScrollingBuffer> m_vec_hist_data;

	UITimer * m_status_timer = nullptr;
	bool m_get_info_req_sent = false;

	char filename[128] = { 0 }; // 单点采集数据文件名

	std::atomic_bool m_start_singlepoint_sampling = false;
};

