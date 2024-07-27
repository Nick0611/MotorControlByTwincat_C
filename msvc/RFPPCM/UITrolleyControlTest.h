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

// ����˶�ģʽ
typedef enum 
{
	Free_mode, // ��
	Jog_mode, // �㶯
	Velocity_mode, // �ٶ�ģʽ
	Position_mode, // λ��ģʽ
	Continus_mode // ����ģʽ��ʵʱ����Ŀ��λ��
}Motor_Motion_Mode;

// �����������
typedef struct
{
	bool set_enable; // ʹ��
	bool set_stop; // ʹ��
	bool set_continue; // ʹ��
	bool set_clear; // ʹ��
	short selected_motion_mode; // �˶�ģʽ
	double jog_velocity_slow; // ���ٵ㶯�ٶ�
	double jog_velocity_fast; // ���ٵ㶯�ٶ�
	double velocity; // Ŀ���ٶ�
	bool velocity_exc; // ��ʼִ��Ŀ���ٶ�
	bool position_exc; // ��ʼִ��Ŀ��λ��
	double position; // Ŀ��λ��
}Motor_SetParamaters;

// ���ʹ��״̬
enum PowerCheckboxFlags
{
	St_PowerOn = 1 << 0, // �Ѿ�ʹ��
	St_PowerOff = 1 >> 1 // δʹ��
};

// �������״̬
enum MotorErrorStatusFlags
{
	St_Normal = 1 << 0, // ����
	St_Abnormal = 1 << 1 // ������
};

// ����˶�״̬
enum MotorMotionStatusFlags
{
	St_MoveDone = 1 << 0, // �˶����
	St_Moving = 1 << 1, // �����˶�
	St_Stoped = 1 << 2 // ��ֹͣ
};

// ���״̬
typedef struct
{
	bool st_power; // �Ƿ�ʹ��
	MotorErrorStatusFlags st_error_status; // ����״̬
	MotorMotionStatusFlags st_motion_status; // ���˶�״̬
	int st_status_error_id; // �����
	double act_position; // ʵ��λ��
	double act_velocity; // ʵ���ٶ�
	double act_acceleration; // ʵ�ʼ��ٶ�
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
private:
	const char* MotorModeNames[5] = {
	"Free_mode",
	"Jog_mode",
	"Velocity_mode",
	"Position_mode",
	"Continus_mode"
	}; // Ϊ�������б������ĵ������ģʽ����
	int m_trolley_num_tmp = 16; // ���ص����������ʱ��
	int m_trolley_num = 16; // ���ص������������ȷ�ϰ�ť����£�
	int m_trolley_num_max = 48; // ��󱻿ص������
	int m_ForceSensor_num_tmp = 4; // ����������������ʱ��
	int m_ForceSensor_num = 4; // ������������������ȷ�ϰ�ť����£�
	int m_ForceSensor_num_max = 16; // ��󴫸�������
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

	std::vector<int> m_slected_force_plot_index; // ѡ��������ͼ�������������
	std::vector<ScrollingBuffer> m_vec_hist_data_force; // ÿ���������������ڻ�ͼ�Ĺ�������������
	std::vector<float> m_vec_hist_data_force_y_max; // ÿ�����������Ļ������е����ֵ
	std::vector<float> m_vec_hist_data_force_y_min; // ÿ�����������Ļ������е���Сֵ
	float m_hist_data_force_y_max; // �������������Ļ������е����ֵ
	float m_hist_data_force_y_min; // �������������Ļ������е���Сֵ
	std::mutex m_vec_hist_data_force_lock; // ����������Ϣ������
	bool m_selected_force_plot_all; // ȫѡ��ѡ�����е����ͼ

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

	std::vector<double> Vel_motor_setvelocity_tmp; // �ٶ��ݴ�ֵ
	std::vector<double> Vel_motor_setposition_tmp; // λ���ݴ�ֵ
	std::vector<int> Con_con_flag; // ִ������ģʽ�ı�־
};

