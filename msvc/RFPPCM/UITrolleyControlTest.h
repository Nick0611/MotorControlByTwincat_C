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
#include <algorithm> // �����㷨������ʹ�� std::max_element
#include <fstream> // дcsv�ļ�
#include "UserTool.h"
#include <thread>
#include <windows.h>
#include "TcAdsDef.h"
#include "TcAdsAPI.h"
#include "AdsExpand.h"

// ����˶�ģʽ
typedef enum 
{
	Free_mode, // ��
	Jog_mode, // �㶯
	Velocity_mode, // �ٶ�ģʽ
	Position_mode, // λ��ģʽ
	Continus_mode, // ����ģʽ��ʵʱ����Ŀ��λ��
	Trajectory_mode	// �켣ģʽ����ȡ�ⲿcsv�켣���켣�ǹؼ��㣬��һ���Ĳ�ֵ��PLC��absolute�˶��н���
}Motor_Motion_Mode;

// �����������
typedef struct
{
	bool set_enable; // ʹ��
	bool set_halt; // ʹ��
	bool set_continue; // ʹ��
	bool set_stop; // ʹ��
	bool set_clear; // ʹ��
	short selected_motion_mode; // �˶�ģʽ
	double jog_velocity_slow; // ���ٵ㶯�ٶ�
	double jog_velocity_fast; // ���ٵ㶯�ٶ�
	double Tarvelocity; // Ŀ���ٶ�
	bool motion_exc; // ��ʼִ��Ŀ���ٶ�
	double Tarposition; // Ŀ��λ��
	double velocity_min; // ��С�ٶ�
	double velocity_max; // ����ٶ�
}Motor_SetParamaters;

// ���ʹ��״̬
enum PowerCheckboxFlags
{
	St_PowerOn = 1 << 0, // �Ѿ�ʹ��
	St_PowerOff = 1 >> 1 // δʹ��
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

// ���״̬
typedef struct
{
	bool st_power; // �Ƿ�ʹ��
	bool st_error_status; // ����״̬
	MC_AxisStates st_motion_status; // �˶�״̬
	unsigned int st_status_error_id; // �����
	double act_position; // ʵ��λ��
	double act_velocity; // ʵ���ٶ�
	double act_acceleration; // ʵ�ʼ��ٶ�
	bool InTargetPosition; // ����Ŀ��λ��
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
	}; // Ϊ�������б������ĵ������ģʽ����
	int m_trolley_num_tmp = 16; // ���ص����������ʱ��
	int m_trolley_num = 16; // ���ص������������ȷ�ϰ�ť����£�
	int m_trolley_num_max = 48; // ��󱻿ص������
	//int m_ForceSensor_num_tmp = 4; // ����������������ʱ��
	//int m_ForceSensor_num = 4; // ������������������ȷ�ϰ�ť����£�
	//int m_ForceSensor_num_max = 16; // ��󴫸�������
	std::vector<int> m_selected_flag; // ���ص���Ƿ���Ա���������ơ����ƣ�������std::vector<bool>����Ϊ�����Ĳ���bool��
	bool m_selected_flag_ALL; // ȫѡ�������е�m_selected_flag��1
	std::vector<int> m_selected_motion_mode; // ����ÿ������˶�ģʽ
	std::vector<int> m_selected_motion_mode_last; // ����ÿ������˶�ģʽ����һ��ѡ��
	int m_selected_motion_mode_ALL; // ��������ơ���ѡ��ĵ���˶�ģʽ
	std::vector<Motor_SetParamaters> m_motor_params; // ����ÿ��������˶���������
	Motor_SetParamaters m_motor_params_ALL; // ��������ơ��е�ͳһ�ĵ���˶���������
	std::vector<Motor_Status> m_motor_status; // ����ÿ�������״̬����
	std::vector<int> m_power_status; // ������ʾ���ʹ��״̬��1Ϊʹ�ܣ�0Ϊδʹ��
	std::vector<int> m_slected_plot_index; // ѡ��������ͼ�ĵ����ţ�������std::vector<bool>����Ϊ�����Ĳ���bool��1�����i�������ѡ��
	bool m_selected_plot_all; // ȫѡ��ѡ�����е����ͼ
	std::mutex m_vec_hist_data_pos_lock; // ���pos��Ϣ������
	std::vector<ScrollingBuffer> m_vec_hist_data_pos; // ÿ�����������pos��ͼ�Ĺ�������������
	std::mutex m_vec_hist_data_vel_lock; // ���vel��Ϣ������
	std::vector<ScrollingBuffer> m_vec_hist_data_vel; // ÿ�����������vel��ͼ�Ĺ�������������
	std::mutex m_vec_hist_data_acc_lock; // ���acc��Ϣ������
	std::vector<ScrollingBuffer> m_vec_hist_data_acc; // ÿ�����������acc��ͼ�Ĺ�������������
	std::vector<float> m_vec_hist_data_pos_y_max; // ÿ�������pos�������е����ֵ
	std::vector<float> m_vec_hist_data_pos_y_min; // ÿ�������pos�������е���Сֵ
	float m_hist_data_pos_y_max; // ���е����pos�������е����ֵ
	float m_hist_data_pos_y_min; // ���е����pos�������е���Сֵ
	std::vector<float> m_vec_hist_data_vel_y_max; // ÿ�������vel�������е����ֵ
	std::vector<float> m_vec_hist_data_vel_y_min; // ÿ�������vel�������е���Сֵ
	float m_hist_data_vel_y_max; // ���е����vel�������е����ֵ
	float m_hist_data_vel_y_min; // ���е����vel�������е���Сֵ
	std::vector<float> m_vec_hist_data_acc_y_max; // ÿ�������acc�������е����ֵ
	std::vector<float> m_vec_hist_data_acc_y_min; // ÿ�������acc�������е���Сֵ
	float m_hist_data_acc_y_max; // ���е����acc�������е����ֵ
	float m_hist_data_acc_y_min; // ���е����acc�������е���Сֵ
	std::ofstream os; // д��csv�ļ�
	char buf_csv_name[256] = { 0 }; // csv�ļ���
	bool m_record_flag_continuous; // �Ƿ�д���ļ����жϱ�־������д�룩
	bool m_record_flag_jog; // �Ƿ�д���ļ����жϱ�־�����д�룩
	int m_record_jog_num; // ��μ�¼�ļ���

	//std::vector<int> m_slected_force_plot_index; // ѡ��������ͼ�������������
	//std::vector<ScrollingBuffer> m_vec_hist_data_force; // ÿ���������������ڻ�ͼ�Ĺ�������������
	//std::vector<float> m_vec_hist_data_force_y_max; // ÿ�����������Ļ������е����ֵ
	//std::vector<float> m_vec_hist_data_force_y_min; // ÿ�����������Ļ������е���Сֵ
	//float m_hist_data_force_y_max; // �������������Ļ������е����ֵ
	//float m_hist_data_force_y_min; // �������������Ļ������е���Сֵ
	//std::mutex m_vec_hist_data_force_lock; // ����������Ϣ������
	//bool m_selected_force_plot_all; // ȫѡ��ѡ�����е����ͼ

	std::mutex m_thread_read_pos_lock;
	std::mutex m_thread_read_vel_lock;
	std::mutex m_thread_read_acc_lock;
	std::thread thread_read_pos;
	std::thread thread_read_vel;
	std::thread thread_read_acc;
	bool thread_read_pos_stop_flag = false;
	bool thread_read_vel_stop_flag = false;
	bool thread_read_acc_stop_flag = false;

	bool ads_local_or_CX_flag; // false����localģʽ��true����cxģʽ
	long      m_nPort;	//����˿ڱ���
	AmsAddr   m_Addr;		//����AMS��ַ����
	PAmsAddr  m_pAddr = &m_Addr;//����˿ڵ�ַ����
	bool* myArray_set_enable = nullptr;
	int m_ReadFrequency_hz; // ����ads��ȡ��Ƶ��

	std::vector<int> Con_con_flag; // ִ������ģʽ�ı�־

	bool Enable_this_module;

	AdsExpand* m_AdsExpand = nullptr;

	std::mutex m_thread_DoubleQueue_Write_lock;
	std::thread thread_DoubleQueue_Write;
	bool thread_DoubleQueue_Write_stop_flag = false;
	int QueueBufferLen = 10;
	double test_array_ads[10000] = { 0 };

	int motorMotionModeCount = 6; // ��Ϊ��6��ö��ֵ

	bool PressDown_All;
	std::vector<int> PressDown;	// ��ʼ�˶���ť�Ƿ��¹��ı�־

};

