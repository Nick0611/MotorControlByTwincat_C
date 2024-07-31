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
	int m_ForceSensor_num_tmp = 4; // ����������������ʱ��
	int m_ForceSensor_num = 4; // ������������������ȷ�ϰ�ť����£�
	int m_ForceSensor_num_max = 16; // ��󴫸�������
	
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
	bool m_selected_force_plot_all; // ȫѡ��ѡ�����е����ͼ

	std::mutex m_thread_read_force_lock;
	std::thread thread_read_force;
	bool thread_read_force_stop_flag;
	std::mutex m_vec_hist_data_force_lock; // ���pos��Ϣ������

	bool ads_local_or_CX_flag; // false����localģʽ��true����cxģʽ
	long      m_nPort;	//����˿ڱ���
	AmsAddr   m_Addr;		//����AMS��ַ����
	PAmsAddr  m_pAddr = &m_Addr;//����˿ڵ�ַ����
	bool* myArray_set_enable = nullptr;
	int m_ReadFrequency_hz; // ����ads��ȡ��Ƶ��

	bool Enable_this_module;
};

