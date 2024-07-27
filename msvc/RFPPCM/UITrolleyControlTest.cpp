#include "UITrolleyControlTest.h"
#include "GLFW/glfw3.h" // �����cpp�ļ���include
#include <cmath>
#include "nlohmann/json.hpp"
#include "TimeCounter.h"

void UITrolleyControlTest::Thread_ReadActPos()
{
	long      nErr, nPort;	//����˿ڱ���
	AmsAddr   Addr;		//����AMS��ַ����
	PAmsAddr  pAddr = &Addr;//����˿ڵ�ַ����
	unsigned long pcbReturn;
	nPort = AdsPortOpenEx();		//��ADSͨѶ�˿�
	if (ads_local_or_CX_flag)
	{
		pAddr->netId.b[0] = m_pAddr->netId.b[0];  //�ֶ���дĿ���豸��AMSNETID
		pAddr->netId.b[1] = m_pAddr->netId.b[1]; //���������豸ͨѶAMSNETIDΪ5.23.20.78.1.1
		pAddr->netId.b[2] = m_pAddr->netId.b[2];
		pAddr->netId.b[3] = m_pAddr->netId.b[3];
		pAddr->netId.b[4] = m_pAddr->netId.b[4];
		pAddr->netId.b[5] = m_pAddr->netId.b[5];
	}
	else
	{
		long nErr = AdsGetLocalAddressEx(nPort, pAddr); //�Զ���ȡ���ص�ַ
		if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
	}
	pAddr->port = 851;			//TC3��ͨѶ�˿�Ϊ851
	unsigned long lHdlVar;   	//�������
	char szVar_pos[] = { "main.pos" };
	while (!thread_read_pos_stop_flag)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		float* myArray = new float[m_trolley_num];
		nErr = AdsSyncReadWriteReqEx2(nPort, pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar_pos), szVar_pos, &pcbReturn);
		if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
		{
			// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
			nErr = AdsSyncReadReqEx2(nPort, pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(float)* m_trolley_num, &myArray[0], &pcbReturn); //��������ǹ̶���С��������
			if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
			else
			{
				std::lock_guard<std::mutex> lock(m_thread_read_pos_lock);
				for (unsigned int i = 0; i < m_trolley_num; i++)//��forѭ�������ʵ�ֶ�ȡ�����е�Ԫ��
				{
					float t = (float)glfwGetTime();
					m_vec_hist_data_pos[i].AddPoint(t, myArray[i]);
				}
			}
		}
		delete[] myArray;
		auto endTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		int sleepTime = int(1000 / m_ReadFrequency_hz) - static_cast<int>(elapsedTime);
		if (sleepTime > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
		}
	}
	AdsPortCloseEx(nPort);
}
void UITrolleyControlTest::Thread_ReadActVel()
{
	long      nErr, nPort;	//����˿ڱ���
	AmsAddr   Addr;		//����AMS��ַ����
	PAmsAddr  pAddr = &Addr;//����˿ڵ�ַ����
	unsigned long pcbReturn;
	nPort = AdsPortOpenEx();		//��ADSͨѶ�˿�
	if (ads_local_or_CX_flag)
	{
		pAddr->netId.b[0] = m_pAddr->netId.b[0];  //�ֶ���дĿ���豸��AMSNETID
		pAddr->netId.b[1] = m_pAddr->netId.b[1]; //���������豸ͨѶAMSNETIDΪ5.23.20.78.1.1
		pAddr->netId.b[2] = m_pAddr->netId.b[2];
		pAddr->netId.b[3] = m_pAddr->netId.b[3];
		pAddr->netId.b[4] = m_pAddr->netId.b[4];
		pAddr->netId.b[5] = m_pAddr->netId.b[5];
	}
	else
	{
		long nErr = AdsGetLocalAddressEx(nPort, pAddr); //�Զ���ȡ���ص�ַ
		if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
	}
	pAddr->port = 851;			//TC3��ͨѶ�˿�Ϊ851
	unsigned long lHdlVar;   	//�������
	char szVar_vel[] = { "main.vel" };
	while (!thread_read_vel_stop_flag)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		float* myArray = new float[m_trolley_num];
		nErr = AdsSyncReadWriteReqEx2(nPort, pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar_vel), szVar_vel, &pcbReturn);
		if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
		{
			nErr = AdsSyncReadReqEx2(nPort, pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(float) * m_trolley_num, &myArray[0], &pcbReturn); //��������ǹ̶���С��������
			if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
			else
			{
				std::lock_guard<std::mutex> lock(m_thread_read_vel_lock);
				for (unsigned int i = 0; i < m_trolley_num; i++)//��forѭ�������ʵ�ֶ�ȡ�����е�Ԫ��
				{
					float t = (float)glfwGetTime();
					m_vec_hist_data_vel[i].AddPoint(t, myArray[i]);
				}
			}
		}
		delete[] myArray;
		auto endTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		int sleepTime = int(1000 / m_ReadFrequency_hz) - static_cast<int>(elapsedTime);
		if (sleepTime > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
		}
	}
	AdsPortCloseEx(nPort);
}
void UITrolleyControlTest::Thread_ReadActAcc()
{
	long      nErr, nPort;	//����˿ڱ���
	AmsAddr   Addr;		//����AMS��ַ����
	PAmsAddr  pAddr = &Addr;//����˿ڵ�ַ����
	unsigned long pcbReturn;
	nPort = AdsPortOpenEx();		//��ADSͨѶ�˿�
	if (ads_local_or_CX_flag)
	{
		pAddr->netId.b[0] = m_pAddr->netId.b[0];  //�ֶ���дĿ���豸��AMSNETID
		pAddr->netId.b[1] = m_pAddr->netId.b[1]; //���������豸ͨѶAMSNETIDΪ5.23.20.78.1.1
		pAddr->netId.b[2] = m_pAddr->netId.b[2];
		pAddr->netId.b[3] = m_pAddr->netId.b[3];
		pAddr->netId.b[4] = m_pAddr->netId.b[4];
		pAddr->netId.b[5] = m_pAddr->netId.b[5];
	}
	else
	{
		long nErr = AdsGetLocalAddressEx(nPort, pAddr); //�Զ���ȡ���ص�ַ
		if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
	}
	pAddr->port = 851;			//TC3��ͨѶ�˿�Ϊ851
	unsigned long lHdlVar;   	//�������
	char szVar_acc[] = { "main.acc" };

	while (!thread_read_acc_stop_flag)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		float* myArray = new float[m_trolley_num];
		nErr = AdsSyncReadWriteReqEx2(nPort, pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar_acc), szVar_acc, &pcbReturn);
		if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
		{
			nErr = AdsSyncReadReqEx2(nPort, pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(float) * m_trolley_num, &myArray[0], &pcbReturn); //��������ǹ̶���С��������
			if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
			else
			{
				std::lock_guard<std::mutex> lock(m_thread_read_acc_lock);
				for (unsigned int i = 0; i < m_trolley_num; i++)//��forѭ�������ʵ�ֶ�ȡ�����е�Ԫ��
				{
					float t = (float)glfwGetTime();
					m_vec_hist_data_acc[i].AddPoint(t, myArray[i]);
				}
			}
		}
		delete[] myArray;
		auto endTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		int sleepTime = int(1000 / m_ReadFrequency_hz) - static_cast<int>(elapsedTime);
		if (sleepTime > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
		}
	}
	AdsPortCloseEx(nPort);
}
bool UITrolleyControlTest::LoadCfg()
{
	TimeCounter t;
	// parse json configure file
	{
		std::ifstream ifs("AdsCfg.json");
		if (!ifs.is_open())
		{
			UI_INFO("File %s does not exists!\n", "AdsCfg.json");
			return false;
		}
		try
		{
			// ���� JSON ����
			nlohmann::json config;
			ifs >> config;

			m_ReadFrequency_hz = config["ReadFrequency_hz"];
			// ��ȡ "TargetSource" ��ֵ
			// ���� "TargetSource" ��ִֵ����Ӧ�Ĳ���
			std::string targetSource = config["TargetSource"];
			if (targetSource == "Local") {
				// ����������
				ads_local_or_CX_flag = false;
			}
			else if (config.find(targetSource) != config.end()) {
				// ��ȡ��Ӧ���������
				ads_local_or_CX_flag = true;
				m_pAddr->netId.b[0] = config[targetSource]["NetId"]["b1"];
				m_pAddr->netId.b[1] = config[targetSource]["NetId"]["b2"];
				m_pAddr->netId.b[2] = config[targetSource]["NetId"]["b3"];
				m_pAddr->netId.b[3] = config[targetSource]["NetId"]["b4"];
				m_pAddr->netId.b[4] = config[targetSource]["NetId"]["b5"];
				m_pAddr->netId.b[5] = config[targetSource]["NetId"]["b6"];
			}
			else {
				UI_ERROR("No target ads information");
				return false;
			}
		}
		catch (const std::exception& e)
		{
			UI_ERROR("Parsing %s exception: %s\n", "AdsCfg.json", e.what());
			return false;
		}
	}
	t.Tick(__FUNCTION__);
	return true;
}

void UITrolleyControlTest::WriteCsv()
{
	int tmp_count = 0; // �ж��Ƿ���ÿһ�еĵ�һ�����ݣ���һ������Ϊʱ�䣬�ǵ�����
	for (int i = 0; i < m_trolley_num; i++)
	{
		if (m_slected_plot_index[i] > 0)
		{
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_pos_lock);
				if (!m_vec_hist_data_pos[i].Data.empty())
				{
					tmp_count++;
					if (tmp_count == 1)
					{
						if (m_vec_hist_data_pos[i].Data.size() < m_vec_hist_data_pos[i].MaxSize) // �жϹ����������Ƿ���
						{
							os << m_vec_hist_data_pos[i].Data[m_vec_hist_data_pos[i].Data.size() - 1].x << ", " // ���ݵ�һ��Ҫ����ʱ���
								<< m_vec_hist_data_pos[i].Data[m_vec_hist_data_pos[i].Data.size() - 1].y;
						}
						else
						{
							os << m_vec_hist_data_pos[i].Data[m_vec_hist_data_pos[i].Offset].x << ", "
								<< m_vec_hist_data_pos[i].Data[m_vec_hist_data_pos[i].Offset].y;
						}
					}
					else
					{
						if (m_vec_hist_data_pos[i].Data.size() < m_vec_hist_data_pos[i].MaxSize)
						{
							os << "," << m_vec_hist_data_pos[i].Data[m_vec_hist_data_pos[i].Data.size() - 1].y;
						}
						else
						{
							os << ", " << m_vec_hist_data_pos[i].Data[m_vec_hist_data_pos[i].Offset].y;
						}
					}
				}
				else
				{
					UI_WARN(u8"���%dû��λ�����ݣ�", i);
					if (tmp_count == 1)
					{
						os << ", ";
						os << ", ";
					}
					else
					{
						os << ", ";
					}
				}
			}
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_vel_lock);
				if (!m_vec_hist_data_vel[i].Data.empty()) // �жϹ����������Ƿ���
				{
					if (m_vec_hist_data_vel[i].Data.size() < m_vec_hist_data_vel[i].MaxSize)
					{
						os << "," << m_vec_hist_data_vel[i].Data[m_vec_hist_data_vel[i].Data.size() - 1].y;
					}
					else
					{
						os << ", " << m_vec_hist_data_vel[i].Data[m_vec_hist_data_vel[i].Offset].y;
					}
				}
				else
				{
					UI_WARN(u8"���%dû���ٶ����ݣ�", i);
					os << ", ";
				}
			}
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_acc_lock);
				if (!m_vec_hist_data_acc[i].Data.empty()) // �жϹ����������Ƿ���
				{
					if (m_vec_hist_data_acc[i].Data.size() < m_vec_hist_data_acc[i].MaxSize)
					{
						os << "," << m_vec_hist_data_acc[i].Data[m_vec_hist_data_acc[i].Data.size() - 1].y;
					}
					else
					{
						os << ", " << m_vec_hist_data_acc[i].Data[m_vec_hist_data_acc[i].Offset].y;
					}
				}
				else
				{
					UI_WARN(u8"���%dû�м��ٶ����ݣ�", i);
					os << ", ";
				}
			}
			
		}
	}
	for (int i = 0; i < m_ForceSensor_num; i++)
	{
		if (m_slected_force_plot_index[i] > 0)
		{
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
				if (!m_vec_hist_data_force[i].Data.empty())
				{
					tmp_count++;
					if (tmp_count == 1)
					{
						if (m_vec_hist_data_force[i].Data.size() < m_vec_hist_data_force[i].MaxSize) // �жϹ����������Ƿ���
						{
							os << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Data.size() - 1].x << ", " // ���ݵ�һ��Ҫ����ʱ���
								<< m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Data.size() - 1].y;
						}
						else
						{
							os << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Offset].x << ", "
								<< m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Offset].y;
						}
					}
					else
					{
						if (m_vec_hist_data_force[i].Data.size() < m_vec_hist_data_force[i].MaxSize)
						{
							os << "," << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Data.size() - 1].y;
						}
						else
						{
							os << ", " << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Offset].y;
						}
					}
				}
				else
				{
					UI_WARN(u8"��������%dû�����ݣ�", i);
					os << ", ";
				}
			}
		}
	}
	if (tmp_count)
	{
		os << "\n";
	}
}
UITrolleyControlTest::UITrolleyControlTest(UIGLWindow* main_win, const char* title) :UIBaseWindow(main_win, title)
{
	LoadCfg();
	m_selected_flag.resize(m_trolley_num, 0);
	m_selected_motion_mode.resize(m_trolley_num, -1);
	m_selected_motion_mode_last.resize(m_trolley_num, -1);
	m_selected_motion_mode_ALL = -1;
	m_power_status.resize(m_trolley_num, 0);
	m_motor_params.resize(m_trolley_num, { 0 });
	m_motor_status.resize(m_trolley_num, { 0 });
	m_slected_plot_index.resize(m_trolley_num, 0);
	m_vec_hist_data_pos.resize(m_trolley_num);
	m_vec_hist_data_vel.resize(m_trolley_num);
	m_vec_hist_data_acc.resize(m_trolley_num);
	m_vec_hist_data_pos_y_min.resize(m_trolley_num, -1);
	m_vec_hist_data_pos_y_max.resize(m_trolley_num, 1);
	m_hist_data_pos_y_min = -1;
	m_hist_data_pos_y_max = 1;
	m_vec_hist_data_vel_y_min.resize(m_trolley_num, -1);
	m_vec_hist_data_vel_y_max.resize(m_trolley_num, 1);
	m_hist_data_vel_y_min = -1;
	m_hist_data_vel_y_max = 1;
	m_vec_hist_data_acc_y_min.resize(m_trolley_num, -1);
	m_vec_hist_data_acc_y_max.resize(m_trolley_num, 1);
	m_hist_data_acc_y_min = -1;
	m_hist_data_acc_y_max = 1;

	m_slected_force_plot_index.resize(m_ForceSensor_num);
	m_vec_hist_data_force.resize(m_ForceSensor_num);
	m_vec_hist_data_force_y_min.resize(m_ForceSensor_num, -1);
	m_vec_hist_data_force_y_max.resize(m_ForceSensor_num, 1);
	m_hist_data_force_y_min = -1;
	m_hist_data_force_y_min = 1;

	Vel_motor_setvelocity_tmp.resize(m_ForceSensor_num);
	Vel_motor_setposition_tmp.resize(m_ForceSensor_num);
	Con_con_flag.resize(m_ForceSensor_num, 0);

	m_record_flag_continuous = false;
	m_record_flag_jog = false;
	m_record_jog_num = 0;
	m_selected_plot_all = false;
	
	thread_read_pos = std::thread(&UITrolleyControlTest::Thread_ReadActPos, this);
	thread_read_vel = std::thread(&UITrolleyControlTest::Thread_ReadActVel, this);
	thread_read_acc = std::thread(&UITrolleyControlTest::Thread_ReadActAcc, this);
	
	m_nPort = AdsPortOpenEx();		//��ADSͨѶ�˿�
	if (!ads_local_or_CX_flag)
	{
		long nErr = AdsGetLocalAddressEx(m_nPort, m_pAddr); //�Զ���ȡ���ص�ַ
		if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
	}
	m_pAddr->port = 851;			//TC3��ͨѶ�˿�Ϊ851

}

UITrolleyControlTest::~UITrolleyControlTest()
{
	{
		std::lock_guard<std::mutex> lock(m_thread_read_pos_lock);
		thread_read_pos_stop_flag = true;
	}
	{
		std::lock_guard<std::mutex> lock(m_thread_read_vel_lock);
		thread_read_vel_stop_flag = true;
	}
	{
		std::lock_guard<std::mutex> lock(m_thread_read_acc_lock);
		thread_read_acc_stop_flag = true;
	}
	if (thread_read_pos.joinable()) thread_read_pos.join();
	if (thread_read_vel.joinable()) thread_read_vel.join();
	if (thread_read_acc.joinable()) thread_read_acc.join();
	AdsPortCloseEx(m_nPort);
}

void UITrolleyControlTest::Draw()
{
	if (!m_show)
	{
		return;
	}
	if (!ImGui::Begin(m_win_title, &m_show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::End();
		return;
	}

	// 1. ���������
	char buf[256] = { 0 };
	if (ImGui::CollapsingHeader(u8"С���������", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// 1.1 ���������
		ImGui::Spacing();
		ImGui::Spacing(); // �ո�
		ImGui::Indent(); // ����

		if (ImGui::CollapsingHeader(u8"�������", ImGuiTreeNodeFlags_DefaultOpen))
		{
			stbsp_sprintf(buf, u8"ȫѡ");
			bool tmp = ImGui::Checkbox(buf, &m_selected_flag_ALL);
			if (tmp && m_selected_flag_ALL)
			{
				for (int i = 0; i < m_trolley_num; ++i)
				{
					m_selected_flag[i] = 1;
				}
			}
			if (tmp && !m_selected_flag_ALL)
			{
				for (int i = 0; i < m_trolley_num; ++i)
				{
					m_selected_flag[i] = 0;
				}
			}

			// ���ص������������
			ImGui::SameLine();
			if (!m_record_flag_continuous||m_record_flag_jog)
			{
				if (ImGui::SliderInt("Num[1-48]", &m_trolley_num_tmp, 1, m_trolley_num_max)) // ͨ��slider���ñ��ص������
				{

				}
				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s ȷ�ϸ��ĵ������", ICON_FA_MERCURY);
				if (ImGui::Button(buf)) // �����ťȷ����������ֹ�󻬶�
				{
					m_trolley_num = m_trolley_num_tmp;
					m_selected_flag.resize(m_trolley_num);
					m_selected_motion_mode.resize(m_trolley_num);
					m_selected_motion_mode_last.resize(m_trolley_num, -1);
					m_power_status.resize(m_trolley_num);
					m_motor_params.resize(m_trolley_num);
					m_motor_status.resize(m_trolley_num);
					m_slected_plot_index.resize(m_trolley_num);
					m_vec_hist_data_pos.resize(m_trolley_num);
					m_vec_hist_data_vel.resize(m_trolley_num);
					m_vec_hist_data_acc.resize(m_trolley_num);
					m_vec_hist_data_pos_y_min.resize(m_trolley_num, -1);
					m_vec_hist_data_pos_y_max.resize(m_trolley_num, 1);
					m_vec_hist_data_vel_y_min.resize(m_trolley_num, -1);
					m_vec_hist_data_vel_y_max.resize(m_trolley_num, 1);
					m_vec_hist_data_acc_y_min.resize(m_trolley_num, -1);
					m_vec_hist_data_acc_y_max.resize(m_trolley_num, 1);

					Vel_motor_setvelocity_tmp.resize(m_ForceSensor_num);
					Vel_motor_setposition_tmp.resize(m_ForceSensor_num);
					Con_con_flag.resize(m_ForceSensor_num, 0);
				}

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				if (ImGui::SliderInt("Num[1-16]", &m_ForceSensor_num_tmp, 1, m_ForceSensor_num_max)) // ͨ��slider���ñ��ص������
				{

				}
				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s ȷ�ϸ���������������", ICON_FA_MERCURY);
				if (ImGui::Button(buf)) // ����������ťȷ����������ֹ�󻬶�
				{
					m_ForceSensor_num = m_ForceSensor_num_tmp;
					m_slected_force_plot_index.resize(m_ForceSensor_num);
					m_vec_hist_data_force.resize(m_ForceSensor_num);
					m_vec_hist_data_force_y_min.resize(m_ForceSensor_num, -1);
					m_vec_hist_data_force_y_max.resize(m_ForceSensor_num, 1);
				}
			}

			stbsp_sprintf(buf, u8"%s һ�������˶�ģʽ", ICON_FA_LIST);
			if (ImGui::Button(buf)) // �����˶�ģʽѡ��ť
			{
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_selected_motion_mode[i] = m_selected_motion_mode_ALL;
					}
				}
			}

			char preview_value[64] = { 0 };
			if (m_selected_motion_mode_ALL >= 0)
			{
				snprintf(preview_value, sizeof(preview_value), "%s %s", ICON_FA_MICROCHIP, MotorModeNames[m_selected_motion_mode_ALL]);
			}
			ImGui::SameLine();
			if (ImGui::BeginCombo(u8"�˶�ģʽ�б�", preview_value, 0)) // �����˶�ģʽ�����б�
			{
				for (int i = 0; i < 5; i++)
				{
					stbsp_sprintf(buf, u8"%s %s", ICON_FA_MICROCHIP, MotorModeNames[i]);
					if (ImGui::Selectable(buf, m_selected_motion_mode_ALL == i))
					{
						m_selected_motion_mode_ALL = i;
					}
				}
				ImGui::EndCombo();
			}

			stbsp_sprintf(buf, u8"%s һ��ʹ��", ICON_FA_EYE);
			if (ImGui::Button(buf)) // ������ע�ᵽ������Ƶĵ��ʹ��
			{

			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s һ���Ͽ�ʹ��", ICON_FA_EYE_DROPPER); // ������ע�ᵽ������Ƶĵ���Ͽ�ʹ��
			if (ImGui::Button(buf))
			{

			}

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
			stbsp_sprintf(buf, u8"%s һ��ֹͣ", ICON_FA_STOP); // ������ע�ᵽ������Ƶĵ����ͣ
			if (ImGui::Button(buf))
			{

			}
			ImGui::PopStyleColor(4);

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s һ������", ICON_FA_COG); // ������ע�ᵽ������Ƶĵ������
			if (ImGui::Button(buf))
			{

			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s һ���������", ICON_FA_GLASSES); // ������ע�ᵽ������Ƶĵ���������
			if (ImGui::Button(buf))
			{

			}

			switch (m_selected_motion_mode_ALL) // ����ģʽ�б�ѡ��󣬸�����ѡ���˶�ģʽ��ʾ��ͬ�Ŀ���UI
			{
			case Free_mode:
				break;
			case Jog_mode:
				//ImGui::BeginGroup();
			{
				stbsp_sprintf(buf, u8"һ������ȷ��");
				if (ImGui::Button(buf))
				{

				}

				//ImGui::BeginGroup();
				ImGui::SameLine();
				ImGui::InputScalar(u8"�㶯��������", ImGuiDataType_U32, (void*)&m_motor_params_ALL.jog_velocity_slow);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, u8"һ������ȷ��");
				if (ImGui::Button(buf))
				{

				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"�㶯��������", ImGuiDataType_U32, (void*)&m_motor_params_ALL.jog_velocity_fast);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ�����ٷ�ת", ICON_FA_LONG_ARROW_ALT_LEFT);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);
				if (ImGui::IsItemActive()) // ��ⰴ�³���������
				{

				}

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.4f, 0.4f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ�����ٷ�ת", ICON_FA_HAND_POINT_LEFT);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);
				if (ImGui::IsItemActive()) // ��ⰴ�³���������
				{

				}

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.4f, 0.4f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ��������ת", ICON_FA_HAND_POINT_RIGHT);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);
				if (ImGui::IsItemActive()) // ��ⰴ�³���������
				{

				}

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ��������ת", ICON_FA_LONG_ARROW_ALT_RIGHT);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);
				if (ImGui::IsItemActive()) // ��ⰴ�³���������
				{

				}

			}
			//ImGui::EndGroup();
			break;
			case Velocity_mode:
				stbsp_sprintf(buf, u8"%s һ�������ٶ�", ICON_FA_CAR);
				if (ImGui::Button(buf))
				{

				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_U32, (void*)&m_motor_params_ALL.velocity);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ����ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);
				break;
			case Position_mode:
				stbsp_sprintf(buf, u8"%s һ������λ��", ICON_FA_GRIN_STARS);
				if (ImGui::Button(buf))
				{

				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_U32, (void*)&m_motor_params_ALL.position);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s һ�������ٶ�", ICON_FA_CAR);
				if (ImGui::Button(buf))
				{

				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_U32, (void*)&m_motor_params_ALL.velocity);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ����ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);
				break;
			case Continus_mode:
				stbsp_sprintf(buf, u8"%s һ�������ٶ�����", ICON_FA_ROCKET);
				if (ImGui::Button(buf))
				{

				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"�ٶ�����", ImGuiDataType_U32, (void*)&m_motor_params_ALL.velocity);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_U32, (void*)&m_motor_params_ALL.position);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ����ʼ�˶�", ICON_FA_RUNNING);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(10.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(20.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ��ֹͣ�˶�", ICON_FA_SHARE);
				if (ImGui::Button(buf))
				{

				}
				ImGui::PopStyleColor(4);
				break;
			default:
				break;
			}
		}
		ImGui::Separator(); // �ָ�

		// 1.2 ����������
		if (ImGui::CollapsingHeader(u8"��������", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int j = 0; j < m_trolley_num; j++)
			{
				ImGui::PushID(j);
				stbsp_sprintf(buf, u8"���%d", j + 1);
				ImGui::CheckboxFlags(buf, &m_selected_flag[j], 1);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				//ImGui::BeginGroup();
				{
					stbsp_sprintf(buf, u8"%s ʹ��", ICON_FA_EYE);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.SetEnable" };
						m_motor_params[j].set_enable = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i=0;i<m_trolley_num;++i)
						{
							myArray[i] = m_motor_params[i].set_enable;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(bool) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
						}
						delete[] myArray;
					} 

					ImGui::SameLine();
					stbsp_sprintf(buf, u8"%s �Ͽ�ʹ��", ICON_FA_EYE_DROPPER);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.SetEnable" };
						m_motor_params[j].set_enable = false;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].set_enable;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(bool) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
						}
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ֹͣ", ICON_FA_STOP);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.SetStop" };
						m_motor_params[j].set_stop = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].set_stop;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(bool) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
						}
						delete[] myArray;
					}
					ImGui::PopStyleColor(4);

					ImGui::SameLine();
					stbsp_sprintf(buf, u8"%s ����", ICON_FA_COG);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.SetContinue" };
						m_motor_params[j].set_continue = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].set_continue;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(bool) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
						}
						delete[] myArray;
					}

					ImGui::SameLine();
					stbsp_sprintf(buf, u8"%s �������", ICON_FA_GLASSES);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.SetClear" };
						m_motor_params[j].set_clear = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].set_clear;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(bool) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
						}
						delete[] myArray;
					}
				}
				//ImGui::EndGroup();

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				//ImGui::BeginGroup();
				{
					stbsp_sprintf(buf, u8"%s ѡ���˶�ģʽ", ICON_FA_LIST);
					ImGui::Text(buf);

					char preview_value[64] = { 0 };
					if (m_selected_motion_mode[j] >= 0)
					{
						snprintf(preview_value, sizeof(preview_value), "%s %s", ICON_FA_MICROCHIP, MotorModeNames[m_selected_motion_mode[j]]);
					}
					ImGui::SameLine();
					if (ImGui::BeginCombo(u8"�˶�ģʽ�б�", preview_value, 0))
					{
						for (int i = 0; i < 5; i++)
						{
							stbsp_sprintf(buf, u8"%s %s", ICON_FA_MICROCHIP, MotorModeNames[i]);
							if (ImGui::Selectable(buf, m_selected_motion_mode[j] == i)) // �б��е�ѡ����¼��
							{	
								m_selected_motion_mode_last[j] = m_selected_motion_mode[j];
								m_selected_motion_mode[j] = i;
								if (m_selected_motion_mode[j] != m_selected_motion_mode_last[j])
								{
									unsigned long lHdlVar;   	//�������-ʹ��
									unsigned long pcbReturn;
									char szVar[] = { "main.m_SelectedMotionMode" };
									m_motor_params[j].selected_motion_mode = m_selected_motion_mode[j];
									short* myArray = new short[m_trolley_num];
									for (int i = 0; i < m_trolley_num; ++i)
									{
										myArray[i] = m_motor_params[i].selected_motion_mode;
									}
									long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
									if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
									{
										// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
										nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(short) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
										if (nErr) { UI_ERROR("Error: AdsSyncWriteReq: %ld", nErr); }
									}
									delete[] myArray;
								}
							}
						}
						ImGui::EndCombo();
					}
				}
				//ImGui::EndGroup();

				switch (m_selected_motion_mode[j])
				{
				case Free_mode:
					break;
				case Jog_mode:
					//ImGui::BeginGroup();
				{
					stbsp_sprintf(buf, u8"����ȷ��");
					if (ImGui::Button(buf))
					{

					}

					//ImGui::BeginGroup();
					ImGui::SameLine();
					ImGui::InputScalar(u8"�㶯��������", ImGuiDataType_U32, (void*)&m_motor_params[j].jog_velocity_slow);

					ImGui::SameLine();
					stbsp_sprintf(buf, " | ");
					ImGui::Text(buf);

					ImGui::SameLine();
					stbsp_sprintf(buf, u8"����ȷ��");
					if (ImGui::Button(buf))
					{

					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"�㶯��������", ImGuiDataType_U32, (void*)&m_motor_params[j].jog_velocity_fast);

					ImGui::SameLine();
					stbsp_sprintf(buf, " | ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ���ٷ�ת", ICON_FA_LONG_ARROW_ALT_LEFT);
					if (ImGui::Button(buf))
					{

					}
					ImGui::PopStyleColor(4);
					if (ImGui::IsItemActive()) // ��ⰴ�³���������
					{

					}

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.4f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ���ٷ�ת", ICON_FA_HAND_POINT_LEFT);
					if (ImGui::Button(buf))
					{

					}
					ImGui::PopStyleColor(4);
					if (ImGui::IsItemActive()) // ��ⰴ�³���������
					{

					}

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.4f, 0.4f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ������ת", ICON_FA_HAND_POINT_RIGHT);
					if (ImGui::Button(buf))
					{

					}
					ImGui::PopStyleColor(4);
					if (ImGui::IsItemActive()) // ��ⰴ�³���������
					{

					}

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(40.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(50.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(60.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ������ת", ICON_FA_LONG_ARROW_ALT_RIGHT);
					if (ImGui::Button(buf))
					{

					}
					ImGui::PopStyleColor(4);
					if (ImGui::IsItemActive()) // ��ⰴ�³���������
					{

					}

				}
				//ImGui::EndGroup();
				break;
				case Velocity_mode:
					stbsp_sprintf(buf, u8"%s �����ٶ�", ICON_FA_CAR);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Vel" };
						m_motor_params[j].velocity = Vel_motor_setvelocity_tmp[j];
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].velocity;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(double) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncWriteReq: %ld", nErr); }
						}
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_Double, (void*)&Vel_motor_setvelocity_tmp[j]);

					ImGui::SameLine();
					stbsp_sprintf(buf, " | ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_MoveVel_Exc" };
						m_motor_params[j].velocity_exc = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].velocity_exc;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(bool) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncWriteReq: %ld", nErr); }
						}
						delete[] myArray;
					}
					ImGui::PopStyleColor(4);
					break;
				case Position_mode:λ��ģʽ������ģʽ�������ٶȻ��ǵ÷ֿ�
					stbsp_sprintf(buf, u8"%s ����λ��", ICON_FA_GRIN_STARS);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Pos" };
						m_motor_params[j].position = Vel_motor_setposition_tmp[j];
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].position;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(double) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncWriteReq: %ld", nErr); }
						}
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_Double, (void*)&Vel_motor_setposition_tmp[j]);

					ImGui::SameLine();
					stbsp_sprintf(buf, " | ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::SameLine();
					stbsp_sprintf(buf, u8"%s �����ٶ�", ICON_FA_CAR);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Vel" };
						m_motor_params[j].velocity = Vel_motor_setvelocity_tmp[j];
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].velocity;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(double) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncWriteReq: %ld", nErr); }
						}
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_Double, (void*)&Vel_motor_setvelocity_tmp[j]);

					ImGui::SameLine();
					stbsp_sprintf(buf, " | ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_MovePos_Exc" };
						m_motor_params[j].position_exc = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].position_exc;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(bool) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
						}
						delete[] myArray;
					}
					ImGui::PopStyleColor(4);
					break;
				case Continus_mode:
					stbsp_sprintf(buf, u8"%s �����ٶ�����", ICON_FA_ROCKET);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Vel" };
						m_motor_params[j].velocity = Vel_motor_setvelocity_tmp[j];
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].velocity;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(double) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncWriteReq: %ld", nErr); }
						}
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"�ٶ�����", ImGuiDataType_Double, (void*)&Vel_motor_setvelocity_tmp[j]);

					ImGui::SameLine();
					stbsp_sprintf(buf, " | ");
					ImGui::Text(buf);

					ImGui::SameLine();
					// ImGuiInputTextFlags_EnterReturnsTrue ʹ�������־����������Ҫ��һ�»س����ܸ�������
					ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_Double, (void*)&m_motor_params[j].position, NULL, NULL, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);

					ImGui::SameLine();
					stbsp_sprintf(buf, " | ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ʼ�˶�", ICON_FA_RUNNING);
					if (ImGui::Button(buf))
					{
						Con_con_flag[j] = 1;
					}
					ImGui::PopStyleColor(4);
					if (Con_con_flag[j])
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Pos" };
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].position;
						}
						long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar), szVar, &pcbReturn);
						if (nErr) UI_ERROR("Error: AdsSyncReadWriteReq: %ld", nErr);
						{
							// ע�⣺����ֱ��sizeof(myArray)����С���ԣ�����
							nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(double) * m_trolley_num, &myArray[0]); //���д���ǹ̶���С��������
							if (nErr) { UI_ERROR("Error: AdsSyncWriteReq: %ld", nErr); }
						}
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(10.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(20.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ֹͣ�˶�", ICON_FA_SHARE);
					if (ImGui::Button(buf))
					{
						Con_con_flag[j] = 0;
					}
					ImGui::PopStyleColor(4);
					break;
				default:
					break;
				}
				switch (m_motor_status[j].st_power)
				{
				case St_PowerOn:
					m_power_status[j] = 1;
					break;
				case St_PowerOff:
					m_power_status[j] = 0;
					break;
				default: // Ҫ����defaultѡ���Ϊÿ��ˢ�����ڶ�Ҫ��m_power_status��ֵ���������ֵ�������checkbox���Ա����������
					m_power_status[j] = 0;
					break;
				}
				ImGui::CheckboxFlags(u8"ʹ��״̬", &m_power_status[j], St_PowerOn);

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s ���״̬:", ICON_FA_PASTAFARIANISM);
				ImGui::Text(buf);

				ImGui::SameLine();
				switch (m_motor_status[j].st_error_status)
				{
				case St_Normal:
					stbsp_sprintf(buf, u8"����");
					ImGui::Text(buf);
					break;
				case St_Abnormal:
					stbsp_sprintf(buf, u8"�쳣");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::InputScalar(u8"������", ImGuiDataType_U32, &m_motor_status[j].st_status_error_id);
					break;
				default:
					stbsp_sprintf(buf, u8"δ֪");
					ImGui::Text(buf);
					break;
				}

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s �˶�״̬:", ICON_FA_PASTAFARIANISM);
				ImGui::Text(buf);

				ImGui::SameLine();
				switch (m_motor_status[j].st_motion_status)
				{
				case St_MoveDone:
					stbsp_sprintf(buf, u8"�˶����");
					ImGui::Text(buf);
					break;
				case St_Moving:
					stbsp_sprintf(buf, u8"�����˶�");
					ImGui::Text(buf);
					break;
				case St_Stoped:
					stbsp_sprintf(buf, u8"�˶��ж�");
					ImGui::Text(buf);
					break;
				default:
					stbsp_sprintf(buf, u8"���˶�");
					ImGui::Text(buf);
					break;
				}

				ImGui::SameLine();
				stbsp_sprintf(buf, " | ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::InputScalar(u8"��ǰλ��", ImGuiDataType_Double, &m_motor_status[j].act_position);
				ImGui::SameLine();
				ImGui::InputScalar(u8"��ǰ�ٶ�", ImGuiDataType_Double, &m_motor_status[j].act_velocity);
				ImGui::SameLine();
				ImGui::InputScalar(u8"��ǰ���ٶ�", ImGuiDataType_Double, &m_motor_status[j].act_acceleration);

				ImGui::Spacing(); // �ո�
				ImGui::Spacing(); // �ո�
				ImGui::Spacing(); // �ո�
				ImGui::Separator(); // �ָ�
				ImGui::Spacing(); // �ո�
				ImGui::Spacing(); // �ո�
				ImGui::Spacing(); // �ո�

				ImGui::PopID();

			}
		}

		ImGui::Unindent();
	}

	if (ImGui::CollapsingHeader(u8"С����ϸ��Ϣ", ImGuiTreeNodeFlags_DefaultOpen))
	{
		stbsp_sprintf(buf, u8"%s ѡ����ID", ICON_FA_LIST);
		ImGui::Text(buf);

		// ѡ�����еĵ������ͼ
		ImGui::SameLine();
		ImGui::PushID(100);
		stbsp_sprintf(buf, u8"ȫѡ");
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		bool tmp1 = ImGui::Checkbox(buf, &m_selected_plot_all);
		if (tmp1 && m_selected_plot_all)
		{
			for (int i = 0; i < m_trolley_num; ++i)
			{
				m_slected_plot_index[i] = 1;
			}
		}
		if (tmp1 && !m_selected_plot_all)
		{
			for (int i = 0; i < m_trolley_num; ++i)
			{
				m_slected_plot_index[i] = 0;
			}
		}
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PopItemFlag();
		}
		ImGui::PopID();

		// ���checkbox���������ʾ�ᳬ����Ļ�������¸���
		float checkboxWidth = 100.0f;  // ÿ�� Checkbox �Ŀ��
		const int minColumns = 1;  // ÿ����С����
		const float padding = 30.0;  // Checkbox ֮��ļ��
		ImVec2 contentRegionMax = ImGui::GetWindowContentRegionMax();
		float availableWidth = contentRegionMax.x - padding;
		int numColumns = max(static_cast<int>(availableWidth / (checkboxWidth + padding)), minColumns);
		ImGui::Columns(numColumns, "CheckboxColumns", false);
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		for (int i = 0; i < m_trolley_num; ++i) {
			ImGui::CheckboxFlags((u8"��� " + std::to_string(i + 1)).c_str(), &m_slected_plot_index[i], 1);
			ImGui::NextColumn();
		}
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PopItemFlag();
		}

		ImGui::Columns(1);  // �ָ�Ϊ���в���

		stbsp_sprintf(buf, u8"%s ѡ����������ID", ICON_FA_LIST);
		ImGui::Text(buf);

		// ѡ�����е�������������ͼ
		ImGui::SameLine();
		ImGui::PushID(1000);
		stbsp_sprintf(buf, u8"ȫѡ");
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		bool tmp2 = ImGui::Checkbox(buf, &m_selected_force_plot_all);
		if (tmp2 && m_selected_force_plot_all)
		{
			for (int i = 0; i < m_ForceSensor_num; ++i)
			{
				m_slected_force_plot_index[i] = 1;
			}
		}
		if (tmp2 && !m_selected_force_plot_all)
		{
			for (int i = 0; i < m_ForceSensor_num; ++i)
			{
				m_slected_force_plot_index[i] = 0;
			}
		}
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PopItemFlag();
		}
		ImGui::PopID();

		checkboxWidth = 150.0f;  // ÿ�� Checkbox �Ŀ��
		numColumns = max(static_cast<int>(availableWidth / (checkboxWidth + padding)), minColumns);
		ImGui::Columns(numColumns, "CheckboxColumns", false);
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		for (int i = 0; i < m_ForceSensor_num; ++i) {
			ImGui::CheckboxFlags((u8"�������� " + std::to_string(i + 1)).c_str(), &m_slected_force_plot_index[i], 1);
			ImGui::NextColumn();
		}
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PopItemFlag();
		}
		ImGui::Columns(1);  // �ָ�Ϊ���в���

		if (m_record_flag_jog)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		stbsp_sprintf(buf, u8"%s ��ʼ������¼", ICON_FA_MINUS_CIRCLE);
		if (ImGui::Button(buf))
		{
			char dt[64] = { 0 };
			GetTimeStr(dt, sizeof(dt));
			stbsp_snprintf(buf_csv_name, sizeof(buf_csv_name), "Motor_info_selected_%s.csv", dt);
			os.open(buf_csv_name);
			if (!os.is_open())
			{
				UI_WARN(u8"���ļ�%sʧ�ܣ�", buf_csv_name);
			}
			else
			{
				m_record_flag_continuous = true;
				UI_INFO(u8"��ʼ������¼");
				os << u8"ʱ��";
				for (int i = 0; i < m_trolley_num; i++)
				{
					if (m_slected_plot_index[i] > 0) // ������ѡ�����Ҫ��ͼ�ĵ�����������ͷ
					{
						os << "," << (u8"���" + std::to_string(i + 1) + u8"λ��") << "," << (u8"���" + std::to_string(i + 1) + u8"�ٶ�") << ","
							<< (u8"���" + std::to_string(i + 1) + u8"���ٶ�");
					}
				}
				for (int i = 0; i < m_ForceSensor_num; i++)
				{
					if (m_slected_force_plot_index[i] > 0) // ������ѡ�����Ҫ��ͼ�������������������ͷ
					{
						os << "," << u8"��������" + std::to_string(i + 1);
					}
				}
				os << "\n";
			}
		}
		if (m_record_flag_jog)
		{
			ImGui::PopItemFlag();
		}

		if (m_record_flag_continuous) // ��¼���ݵ�csv�ļ�
		{
			WriteCsv();
		}

		if (m_record_flag_jog)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		ImGui::SameLine();
		stbsp_sprintf(buf, u8"%s ֹͣ������¼", ICON_FA_MINUS_SQUARE);
		if (ImGui::Button(buf))
		{
			if (os.is_open())
			{
				UI_INFO(u8"������ѡ��ĵ�����ݵ��ļ�%s�ɹ���", buf_csv_name);
				os.close();
				m_record_flag_continuous = false;
			}
		}
		if (m_record_flag_jog)
		{
			ImGui::PopItemFlag();
		}

		ImGui::SameLine();
		stbsp_sprintf(buf, u8"%s ��ʼ��μ�¼", ICON_FA_MINUS_CIRCLE);
		if (m_record_flag_continuous)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		if (ImGui::Button(buf))
		{
			if (m_record_flag_jog)
			{
				m_record_jog_num++;
				WriteCsv();
				UI_INFO(u8"�� %d �μ�¼", m_record_jog_num);
			}
			else
			{
				char dt[64] = { 0 };
				GetTimeStr(dt, sizeof(dt));
				stbsp_snprintf(buf_csv_name, sizeof(buf_csv_name), "Motor_info_selected_%s.csv", dt);
				os.open(buf_csv_name);
				if (!os.is_open())
				{
					UI_WARN(u8"���ļ�%sʧ�ܣ�", buf_csv_name);
				}
				else
				{
					m_record_jog_num = 0;
					m_record_flag_jog = true;
					UI_INFO(u8"�� %d �μ�¼", m_record_jog_num);
					os << u8"ʱ��";
					for (int i = 0; i < m_trolley_num; i++)
					{
						if (m_slected_plot_index[i] > 0) // ������ѡ�����Ҫ��ͼ�ĵ�����������ͷ
						{
							os << "," << (u8"���" + std::to_string(i + 1) + u8"λ��") << "," << (u8"���" + std::to_string(i + 1) + u8"�ٶ�") << ","
								<< (u8"���" + std::to_string(i + 1) + u8"���ٶ�");
						}
					}
					for (int i = 0; i < m_ForceSensor_num; i++)
					{
						if (m_slected_force_plot_index[i] > 0) // ������ѡ�����Ҫ��ͼ�������������������ͷ
						{
							os << "," << u8"��������" + std::to_string(i + 1);
						}
					}
					os << "\n";
				}
			}
		}
		if (m_record_flag_continuous)
		{
			ImGui::PopItemFlag();
		}

		ImGui::SameLine();
		if (m_record_flag_continuous)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		stbsp_sprintf(buf, u8"%s ֹͣ��μ�¼", ICON_FA_MINUS_SQUARE);
		if (ImGui::Button(buf))
		{
			if (os.is_open())
			{
				UI_INFO(u8"������ѡ��ĵ�����ݵ��ļ�%s�ɹ���", buf_csv_name);
				os.close();
				m_record_flag_jog = false;
			}
		}
		if (m_record_flag_continuous)
		{
			ImGui::PopItemFlag();
		}


		static float history = 10.0f; // ʱ������СΪ-10
		ImGui::SliderFloat(u8"Plotʱ��", &history, 1, 30, "%.1f s"); // ��ͼʱ�䴰������
		static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
		float t = (float)glfwGetTime();
		if (ImPlot::BeginPlot(u8"���λ������", ImVec2(-1, 350)))
		{
			{
				// ��������������е������Сֵ������axis limit����
				std::lock_guard<std::mutex> lock(m_vec_hist_data_pos_lock);
				m_hist_data_pos_y_min = FLT_MAX;
				m_hist_data_pos_y_max = -FLT_MAX;
				for (unsigned int i = 0; i < m_trolley_num; i++)
				{
					if (!m_vec_hist_data_pos[i].Data.empty()) // �жϹ����������Ƿ���
					{
						float max_y_value = std::max_element(m_vec_hist_data_pos[i].Data.begin(), m_vec_hist_data_pos[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
							return a.y < b.y;
							})->y;
						float min_y_value = std::min_element(m_vec_hist_data_pos[i].Data.begin(), m_vec_hist_data_pos[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
							return a.y < b.y;
							})->y;
						m_vec_hist_data_pos_y_max[i] = max_y_value;
						m_vec_hist_data_pos_y_min[i] = min_y_value;
						if (m_slected_plot_index[i])
						{
							m_hist_data_pos_y_min = min(m_vec_hist_data_pos_y_min[i], m_hist_data_pos_y_min);
							m_hist_data_pos_y_max = max(m_vec_hist_data_pos_y_max[i], m_hist_data_pos_y_max);
						}
					}
				}
			}

			ImPlot::SetupAxes("t(s)", "Position", flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always); // ����x�����ʱ�����
			// ���ݼ�ֵ����y��limit
			if (m_hist_data_pos_y_max == m_hist_data_pos_y_min)
			{
				ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_pos_y_min - 1, m_hist_data_pos_y_max + 1, ImGuiCond_Always);
			}
			else
			{
				ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_pos_y_min - (m_hist_data_pos_y_max - m_hist_data_pos_y_min) / 10, m_hist_data_pos_y_max + (m_hist_data_pos_y_max - m_hist_data_pos_y_min) / 10, ImGuiCond_Always);
			}
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_pos_lock);
				for (unsigned int i = 0; i < m_trolley_num; i++)
				{
					if (m_slected_plot_index[i])
					{
						if (!m_vec_hist_data_pos[i].Data.empty()) // �жϹ����������Ƿ���
						{
							ImPlot::PlotLine((u8"��� " + std::to_string(i + 1)).c_str(), &m_vec_hist_data_pos[i].Data[0].x, &m_vec_hist_data_pos[i].Data[0].y,
								m_vec_hist_data_pos[i].Data.size(), m_vec_hist_data_pos[i].Offset, 2 * sizeof(float));
						}
					}
				}
			}
			ImPlot::EndPlot();
		}
		{
			std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
			for (unsigned int i = 0; i < m_ForceSensor_num; i++)
			{
				float t = (float)glfwGetTime();
				m_vec_hist_data_force[i].AddPoint(t, std::sin(t));
			}
		}


		if (ImPlot::BeginPlot(u8"����ٶ�����", ImVec2(-1, 350)))
		{
			{
				// ��������������е������Сֵ������axis limit����
				std::lock_guard<std::mutex> lock(m_vec_hist_data_vel_lock);
				m_hist_data_vel_y_min = FLT_MAX;
				m_hist_data_vel_y_max = -FLT_MAX;
				for (unsigned int i = 0; i < m_trolley_num; i++)
				{
					if (!m_vec_hist_data_vel[i].Data.empty()) // �жϹ����������Ƿ���
					{
						float max_y_value = std::max_element(m_vec_hist_data_vel[i].Data.begin(), m_vec_hist_data_vel[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
							return a.y < b.y;
							})->y;
						float min_y_value = std::min_element(m_vec_hist_data_vel[i].Data.begin(), m_vec_hist_data_vel[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
							return a.y < b.y;
							})->y;
						m_vec_hist_data_vel_y_max[i] = max_y_value;
						m_vec_hist_data_vel_y_min[i] = min_y_value;
						if (m_slected_plot_index[i])
						{
							m_hist_data_vel_y_min = min(m_vec_hist_data_vel_y_min[i], m_hist_data_vel_y_min);
							m_hist_data_vel_y_max = max(m_vec_hist_data_vel_y_max[i], m_hist_data_vel_y_max);
						}
					}
				}
			}

			ImPlot::SetupAxes("t(s)", "Velocity", flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always); // ����x�����ʱ�����
			// ���ݼ�ֵ����y��limit
			if (m_hist_data_vel_y_max == m_hist_data_vel_y_min)
			{
				ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_vel_y_min - 1, m_hist_data_vel_y_max + 1, ImGuiCond_Always);
			}
			else
			{
				ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_vel_y_min - (m_hist_data_vel_y_max - m_hist_data_vel_y_min) / 10, m_hist_data_vel_y_max + (m_hist_data_vel_y_max - m_hist_data_vel_y_min) / 10, ImGuiCond_Always);
			}
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_vel_lock);
				for (unsigned int i = 0; i < m_trolley_num; i++)
				{
					if (m_slected_plot_index[i])
					{
						if (!m_vec_hist_data_vel[i].Data.empty()) // �жϹ����������Ƿ���
						{
							ImPlot::PlotLine((u8"��� " + std::to_string(i + 1)).c_str(), &m_vec_hist_data_vel[i].Data[0].x, &m_vec_hist_data_vel[i].Data[0].y,
								m_vec_hist_data_vel[i].Data.size(), m_vec_hist_data_vel[i].Offset, 2 * sizeof(float));
						}
					}
				}
			}
			ImPlot::EndPlot();
		}

		//if (ImPlot::BeginPlot(u8"������ٶ�����", ImVec2(-1, 350)))
		//{
		//	{
		//		// ��������������е������Сֵ������axis limit����
		//		std::lock_guard<std::mutex> lock(m_vec_hist_data_acc_lock);
		//		m_hist_data_acc_y_min = FLT_MAX;
		//		m_hist_data_acc_y_max = -FLT_MAX;
		//		for (unsigned int i = 0; i < m_trolley_num; i++)
		//		{
		//			if (!m_vec_hist_data_acc[i].Data.empty())
		//			{
		//				float max_y_value = std::max_element(m_vec_hist_data_acc[i].Data.begin(), m_vec_hist_data_acc[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
		//					return a.y < b.y;
		//					})->y;
		//				float min_y_value = std::min_element(m_vec_hist_data_acc[i].Data.begin(), m_vec_hist_data_acc[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
		//					return a.y < b.y;
		//					})->y;
		//				m_vec_hist_data_acc_y_max[i] = max_y_value;
		//				m_vec_hist_data_acc_y_min[i] = min_y_value;
		//				if (m_slected_plot_index[i])
		//				{
		//					m_hist_data_acc_y_min = min(m_vec_hist_data_acc_y_min[i], m_hist_data_acc_y_min);
		//					m_hist_data_acc_y_max = max(m_vec_hist_data_acc_y_max[i], m_hist_data_acc_y_max);
		//				}
		//			}
		//		}
		//	}

		//	ImPlot::SetupAxes("t(s)", "Acceleration", flags, flags);
		//	ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always); // ����x�����ʱ�����
		//	// ���ݼ�ֵ����y��limit
		//	if (m_hist_data_acc_y_max == m_hist_data_acc_y_min)
		//	{
		//		ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_acc_y_min - 1, m_hist_data_acc_y_max + 1, ImGuiCond_Always);
		//	}
		//	else
		//	{
		//		ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_acc_y_min - (m_hist_data_acc_y_max - m_hist_data_acc_y_min) / 10, m_hist_data_acc_y_max + (m_hist_data_acc_y_max - m_hist_data_acc_y_min) / 10, ImGuiCond_Always);
		//	}
		//	{
		//		std::lock_guard<std::mutex> lock(m_vec_hist_data_acc_lock);
		//		for (unsigned int i = 0; i < m_trolley_num; i++)
		//		{
		//			if (m_slected_plot_index[i])
		//			{
		//				if (!m_vec_hist_data_acc[i].Data.empty())
		//				{
		//					ImPlot::PlotLine((u8"��� " + std::to_string(i + 1)).c_str(), &m_vec_hist_data_acc[i].Data[0].x, &m_vec_hist_data_acc[i].Data[0].y,
		//						m_vec_hist_data_acc[i].Data.size(), m_vec_hist_data_acc[i].Offset, 2 * sizeof(float));
		//				}
		//			}
		//		}
		//	}
		//	ImPlot::EndPlot();
		//}

		if (ImPlot::BeginPlot(u8"������������", ImVec2(-1, 350)))
		{
			{
				// ��������������е������Сֵ������axis limit����
				std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
				m_hist_data_force_y_min = FLT_MAX;
				m_hist_data_force_y_max = -FLT_MAX;
				for (unsigned int i = 0; i < m_ForceSensor_num; i++)
				{
					if (!m_vec_hist_data_force[i].Data.empty()) // �жϹ����������Ƿ���
					{
						float max_y_value = std::max_element(m_vec_hist_data_force[i].Data.begin(), m_vec_hist_data_force[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
							return a.y < b.y;
							})->y;
						float min_y_value = std::min_element(m_vec_hist_data_force[i].Data.begin(), m_vec_hist_data_force[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
							return a.y < b.y;
							})->y;
						m_vec_hist_data_force_y_max[i] = max_y_value;
						m_vec_hist_data_force_y_min[i] = min_y_value;

						if (m_slected_force_plot_index[i])
						{
							m_hist_data_force_y_min = min(m_vec_hist_data_force_y_min[i], m_hist_data_force_y_min);
							m_hist_data_force_y_max = max(m_vec_hist_data_force_y_max[i], m_hist_data_force_y_max);
						}
					}
				}
			}

			ImPlot::SetupAxes("t(s)", "ForceSensor", flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always); // ����x�����ʱ�����
			// ���ݼ�ֵ����y��limit
			if (m_hist_data_force_y_max == m_hist_data_force_y_min)
			{
				ImGui::PushID(105);
				ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_force_y_min - 1, m_hist_data_force_y_max + 1, ImGuiCond_Always);
				ImGui::PopID();
			}
			else
			{
				ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_force_y_min - (m_hist_data_force_y_max - m_hist_data_force_y_min) / 10, m_hist_data_force_y_max + (m_hist_data_force_y_max - m_hist_data_force_y_min) / 10, ImGuiCond_Always);
			}
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
				for (unsigned int i = 0; i < m_ForceSensor_num; i++)
				{
					if (m_slected_force_plot_index[i])
					{
						if (!m_vec_hist_data_force[i].Data.empty()) // �жϹ����������Ƿ���
						{
							ImPlot::PlotLine((u8"�������� " + std::to_string(i + 1)).c_str(), &m_vec_hist_data_force[i].Data[0].x, &m_vec_hist_data_force[i].Data[0].y,
								m_vec_hist_data_force[i].Data.size(), m_vec_hist_data_force[i].Offset, 2 * sizeof(float));
						}
					}
				}
			}
			ImPlot::EndPlot();
		}
	}

	ImGui::End();
}