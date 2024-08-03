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
#include "AdsExpand.h"

// 电机运动模式
typedef enum 
{
	Free_mode, // 空
	Jog_mode, // 点动
	Velocity_mode, // 速度模式
	Position_mode, // 位置模式
	Continus_mode, // 连续模式，实时跟踪目标位置
	Trajectory_mode	// 轨迹模式，读取外部csv轨迹，轨迹是关键点，进一步的插值在PLC的absolute运动中进行
}Motor_Motion_Mode;

// 电机参数设置
typedef struct
{
	bool set_enable; // 使能
	bool set_halt; // 使能
	bool set_continue; // 使能
	bool set_stop; // 使能
	bool set_clear; // 使能
	short selected_motion_mode; // 运动模式
	double jog_velocity_slow; // 慢速点动速度
	double jog_velocity_fast; // 快速点动速度
	double Tarvelocity; // 目标速度
	bool motion_exc; // 开始执行目标速度
	double Tarposition; // 目标位置
	double velocity_min; // 最小速度
	double velocity_max; // 最大速度
}Motor_SetParamaters;

// 电机使能状态
enum PowerCheckboxFlags
{
	St_PowerOn = 1 << 0, // 已经使能
	St_PowerOff = 1 >> 1 // 未使能
};

enum MC_AxisStates
{
	MC_AXISSTATE_UNDEFINED,
	MC_AXISSTATE_DISABLED,
	MC_AXISSTATE_STANDSTILL,
	MC_AXISSTATE_ERRORSTOP,
	MC_AXISSTATE_STOPPING,
	MC_AXISSTATE_HOMING,
	MC_AXISSTATE_DISCRETEMOTION,
	MC_AXISSTATE_CONTINUOUSMOTION,
	MC_AXISSTATE_SYNCHRONIZEDMOTION
};

// 电机状态
typedef struct
{
	bool st_power; // 是否使能
	bool st_error_status; // 运行状态
	MC_AxisStates st_motion_status; // 运动状态
	unsigned int st_status_error_id; // 错误号
	double act_position; // 实际位置
	double act_velocity; // 实际速度
	double act_acceleration; // 实际加速度
	bool InTargetPosition; // 到达目标位置
}Motor_Status;



class UITrolleyControlTest : public UIBaseWindow
{
public:
	UITrolleyControlTest(UIGLWindow* main_win, const char* title);
	~UITrolleyControlTest();
	virtual void Draw();
	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_TOOL; }
	virtual const char* GetShowShortCut() { return "Ctrl+8"; }
protected:

public:
	bool LoadCfg();
	void WriteCsv();
public:
	void Thread_ReadActPos();
	void Thread_ReadActVel();
	void Thread_ReadActAcc();

	void Thread_m_test();
private:
	const char* MotorModeNames[6] = {
	"Free_mode",
	"Jog_mode",
	"Velocity_mode",
	"Position_mode",
	"Continus_mode",
	"Trajectory_mode"
	}; // 为了下拉列表而构造的电机运行模式数组
	int m_trolley_num_tmp = 16; // 被控电机数量（暂时）
	int m_trolley_num = 16; // 被控电机数量（按下确认按钮后更新）
	int m_trolley_num_max = 48; // 最大被控电机数量
	//int m_ForceSensor_num_tmp = 4; // 力传感器数量（暂时）
	//int m_ForceSensor_num = 4; // 力传感器数量（按下确认按钮后更新）
	//int m_ForceSensor_num_max = 16; // 最大传感器数量
	std::vector<int> m_selected_flag; // 被控电机是否可以被“总体控制”控制，不能用std::vector<bool>，因为里面存的不是bool！
	bool m_selected_flag_ALL; // 全选框，令所有的m_selected_flag置1
	std::vector<int> m_selected_motion_mode; // 储存每个电机运动模式
	std::vector<int> m_selected_motion_mode_last; // 储存每个电机运动模式（上一次选择）
	int m_selected_motion_mode_ALL; // “总体控制”中选择的电机运动模式
	std::vector<Motor_SetParamaters> m_motor_params; // 储存每个电机的运动参数设置
	Motor_SetParamaters m_motor_params_ALL; // “总体控制”中的统一的电机运动参数设置
	std::vector<Motor_Status> m_motor_status; // 储存每个电机的状态参数
	std::vector<int> m_power_status; // 辅助显示电机使能状态，1为使能，0为未使能
	std::vector<int> m_slected_plot_index; // 选择用来画图的电机序号，不能用std::vector<bool>，因为里面存的不是bool！1代表第i个电机被选择。
	bool m_selected_plot_all; // 全选框，选中所有电机画图
	std::mutex m_vec_hist_data_pos_lock; // 电机pos信息互斥锁
	std::vector<ScrollingBuffer> m_vec_hist_data_pos; // 每个电机的用于pos画图的滚动缓存区数据
	std::mutex m_vec_hist_data_vel_lock; // 电机vel信息互斥锁
	std::vector<ScrollingBuffer> m_vec_hist_data_vel; // 每个电机的用于vel画图的滚动缓存区数据
	std::mutex m_vec_hist_data_acc_lock; // 电机acc信息互斥锁
	std::vector<ScrollingBuffer> m_vec_hist_data_acc; // 每个电机的用于acc画图的滚动缓存区数据
	std::vector<float> m_vec_hist_data_pos_y_max; // 每个电机的pos缓存区中的最大值
	std::vector<float> m_vec_hist_data_pos_y_min; // 每个电机的pos缓存区中的最小值
	float m_hist_data_pos_y_max; // 所有电机的pos缓存区中的最大值
	float m_hist_data_pos_y_min; // 所有电机的pos缓存区中的最小值
	std::vector<float> m_vec_hist_data_vel_y_max; // 每个电机的vel缓存区中的最大值
	std::vector<float> m_vec_hist_data_vel_y_min; // 每个电机的vel缓存区中的最小值
	float m_hist_data_vel_y_max; // 所有电机的vel缓存区中的最大值
	float m_hist_data_vel_y_min; // 所有电机的vel缓存区中的最小值
	std::vector<float> m_vec_hist_data_acc_y_max; // 每个电机的acc缓存区中的最大值
	std::vector<float> m_vec_hist_data_acc_y_min; // 每个电机的acc缓存区中的最小值
	float m_hist_data_acc_y_max; // 所有电机的acc缓存区中的最大值
	float m_hist_data_acc_y_min; // 所有电机的acc缓存区中的最小值
	std::ofstream os; // 写入csv文件
	char buf_csv_name[256] = { 0 }; // csv文件名
	bool m_record_flag_continuous; // 是否写入文件的判断标志（连续写入）
	bool m_record_flag_jog; // 是否写入文件的判断标志（逐次写入）
	int m_record_jog_num; // 逐次记录的计数

	//std::vector<int> m_slected_force_plot_index; // 选择用来画图的力传感器序号
	//std::vector<ScrollingBuffer> m_vec_hist_data_force; // 每个力传感器的用于画图的滚动缓存区数据
	//std::vector<float> m_vec_hist_data_force_y_max; // 每个力传感器的缓存区中的最大值
	//std::vector<float> m_vec_hist_data_force_y_min; // 每个力传感器的缓存区中的最小值
	//float m_hist_data_force_y_max; // 所有力传感器的缓存区中的最大值
	//float m_hist_data_force_y_min; // 所有力传感器的缓存区中的最小值
	//std::mutex m_vec_hist_data_force_lock; // 力传感器信息互斥锁
	//bool m_selected_force_plot_all; // 全选框，选中所有电机画图

	std::mutex m_thread_read_pos_lock;
	std::mutex m_thread_read_vel_lock;
	std::mutex m_thread_read_acc_lock;
	std::thread thread_read_pos;
	std::thread thread_read_vel;
	std::thread thread_read_acc;
	bool thread_read_pos_stop_flag = false;
	bool thread_read_vel_stop_flag = false;
	bool thread_read_acc_stop_flag = false;

	bool ads_local_or_CX_flag; // false代表local模式，true代表cx模式
	long      m_nPort;	//定义端口变量
	AmsAddr   m_Addr;		//定义AMS地址变量
	PAmsAddr  m_pAddr = &m_Addr;//定义端口地址变量
	bool* myArray_set_enable = nullptr;
	int m_ReadFrequency_hz; // 主动ads读取的频率

	std::vector<int> Con_con_flag; // 执行连续模式的标志

	bool Enable_this_module;

	AdsExpand* m_AdsExpand = nullptr;

	std::mutex m_thread_DoubleQueue_Write_lock;
	std::thread thread_DoubleQueue_Write;
	bool thread_DoubleQueue_Write_stop_flag = false;
	int QueueBufferLen = 10;
	double test_array_ads[10000] = { 0 };

	int motorMotionModeCount = 6; // 因为有6个枚举值

	bool PressDown_All;
	std::vector<int> PressDown;	// 开始运动按钮是否按下过的标志

};

