#pragma once
#include "UIBaseWindow.h"
#include "CPSFinRayCalcServerDef.h"
#include "EPOS2.h"
#include <mutex>
#include <map>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>
#include <stdio.h>
#include "UIPlotDef.h"

struct UIFinRayShapeData
{
	std::vector<double> x, y;
};
struct UIFinRayRibData
{
	std::vector<double> rib1x, rib1y;
	std::vector<double> rib2x, rib2y;
	std::vector<double> rib3x, rib3y;
	std::vector<double> rib4x, rib4y;
	std::vector<double> rib5x, rib5y;
};

class UIFinRay :
    public UIBaseWindow
{
public:
    UIFinRay(UIGLWindow* main_win, const char* title);

	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_APP; }
	virtual const char* GetShowShortCut() { return ""; }

	virtual void OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len);

	void UpdatePlotData(const std::string& key1, const std::vector<double>& x, const std::vector<double>& y, const std::string& key2, const std::vector<double>& rbx, const std::vector<double>& rby);
	std::vector<std::vector<double >> readMatrixFile(const char* fileName);
protected:
	void OnFinRayCalcResult(uint32_t from_id, const char* data, uint32_t msg_len);

	void ExecuteForceFeedBackPolicy();

	void SendCalcReq();
protected:
	bool m_is_running = false;

	unsigned short m_epos_node_id = 2;
	std::shared_ptr<Device::EPOS2> m_epos2 = nullptr;

	// 计算结果
	std::mutex m_calc_result_lock;
	ST_RRI_Calc_Result m_calc_result = { 0 };
	std::map<std::string, UIFinRayShapeData> m_map_plot_shape_data;
	std::map<std::string, UIFinRayRibData> m_map_plot_rib_data;
	//标定参数
	std::vector<std::vector<double >> calibration_parameter;
	// 感知力
	std::mutex m_vec_hist_data_lock;
	std::vector<ScrollingBuffer> m_vec_hist_data;

	float fd;// desired force
	float initial_t;
	float Kp;
	float Ki;
	float Kd;

	double pi = 3.1415926;

	char filename[128] = { 0 };
	char force_data_file[128] = { 0 };
	FILE* fp = NULL;
	FILE* fp_force = NULL;
	bool is_start_record_time = false;
};

