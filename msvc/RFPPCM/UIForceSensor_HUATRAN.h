#pragma once
#include "UIBaseWindow.h"
#include <vector>
#include <string>
#include <memory>
#include <mutex>
#include "UIPlotDef.h"
//#include "imgui/imgui.h"
#include "implot/implot.h"
#include "imgui/imgui_internal.h"
#include "IconsFontAwesome5.h"
#include "UILog.h"
#include "stb/stb_sprintf.h"
//#include <map>
#include <algorithm> // 包含算法库用于使用 std::max_element
#include <fstream> // 写csv文件
#include "UserTool.h"
#include <thread>
#include <windows.h>
#include "TcAdsDef.h"
#include "TcAdsAPI.h"

class UIForceSensor_HUATRAN : public UIBaseWindow
{
public:
	UIForceSensor_HUATRAN(UIGLWindow* main_win, const char* title);
	~UIForceSensor_HUATRAN();
	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_TOOL; }
	virtual const char* GetShowShortCut() { return "Ctrl+9"; }
protected:

public:
	bool LoadCfg();
	void WriteCsv();
public:
	void Thread_ReadForceData();
private:
	int m_ForceSensor_num_tmp = 4; // 力传感器数量（暂时）
	int m_ForceSensor_num = 4; // 力传感器数量（按下确认按钮后更新）
	int m_ForceSensor_num_max = 16; // 最大传感器数量
	
	std::ofstream os; // 写入csv文件
	char buf_csv_name[256] = { 0 }; // csv文件名
	bool m_record_flag_continuous; // 是否写入文件的判断标志（连续写入）
	bool m_record_flag_jog; // 是否写入文件的判断标志（逐次写入）
	int m_record_jog_num; // 逐次记录的计数

	std::vector<int> m_slected_force_plot_index; // 选择用来画图的力传感器序号
	std::vector<ScrollingBuffer> m_vec_hist_data_force; // 每个力传感器的用于画图的滚动缓存区数据
	std::vector<float> m_vec_hist_data_force_y_max; // 每个力传感器的缓存区中的最大值
	std::vector<float> m_vec_hist_data_force_y_min; // 每个力传感器的缓存区中的最小值
	float m_hist_data_force_y_max; // 所有力传感器的缓存区中的最大值
	float m_hist_data_force_y_min; // 所有力传感器的缓存区中的最小值
	bool m_selected_force_plot_all; // 全选框，选中所有电机画图

	std::mutex m_thread_read_force_lock;
	std::thread thread_read_force;
	bool thread_read_force_stop_flag;
	std::mutex m_vec_hist_data_force_lock; // 电机pos信息互斥锁

	bool ads_local_or_CX_flag; // false代表local模式，true代表cx模式
	long      m_nPort;	//定义端口变量
	AmsAddr   m_Addr;		//定义AMS地址变量
	PAmsAddr  m_pAddr = &m_Addr;//定义端口地址变量
	bool* myArray_set_enable = nullptr;
	int m_ReadFrequency_hz; // 主动ads读取的频率

	bool Enable_this_module;
};

