#include "UITrolleyControlTest.h"
#include "GLFW/glfw3.h" // �����cpp�ļ���include
#include <cmath>
#include "nlohmann/json.hpp"
#include "TimeCounter.h"

// ��ö��ֵת��Ϊ�ַ���
std::string MC_AxisStateToString(MC_AxisStates state)
{
	switch (state)
	{
	case MC_AXISSTATE_UNDEFINED: return          "UNDEFINED         ";
	case MC_AXISSTATE_DISABLED: return           "DISABLED          ";
	case MC_AXISSTATE_STANDSTILL: return         "STANDSTILL        ";
	case MC_AXISSTATE_ERRORSTOP: return          "ERRORSTOP         ";
	case MC_AXISSTATE_STOPPING: return           "STOPPING          ";
	case MC_AXISSTATE_HOMING: return             "HOMING            ";
	case MC_AXISSTATE_DISCRETEMOTION: return     "DISCRETEMOTION    ";
	case MC_AXISSTATE_CONTINUOUSMOTION: return   "CONTINUOUSMOTION  ";
	case MC_AXISSTATE_SYNCHRONIZEDMOTION: return "SYNCHRONIZEDMOTION";
	default: return                              "Unknown State     ";
	}
}

void UITrolleyControlTest::Thread_ReadActPos()
{
	char szVar_pos[] = { "main.pos" };
	AdsExpand mmads(m_pAddr);
	while (!thread_read_pos_stop_flag)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		double* myArray = new double[m_trolley_num];
		mmads.ReadArray(szVar_pos, myArray, m_trolley_num);
		{
			std::lock_guard<std::mutex> lock(m_thread_read_pos_lock);
			for (unsigned int i = 0; i < m_trolley_num; i++)//��forѭ�������ʵ�ֶ�ȡ�����е�Ԫ��
			{
				float t = (float)glfwGetTime();
				m_vec_hist_data_pos[i].AddPoint(t, myArray[i]);
				m_motor_status[i].act_position = myArray[i];
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
}
void UITrolleyControlTest::Thread_ReadActVel()
{
	char szVar_vel[] = { "main.vel" };
	AdsExpand mmads(m_pAddr);
	while (!thread_read_vel_stop_flag)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		double* myArray = new double[m_trolley_num];
		mmads.ReadArray(szVar_vel, myArray, m_trolley_num);
		{
			std::lock_guard<std::mutex> lock(m_thread_read_vel_lock);
			for (unsigned int i = 0; i < m_trolley_num; i++)//��forѭ�������ʵ�ֶ�ȡ�����е�Ԫ��
			{
				float t = (float)glfwGetTime();
				m_vec_hist_data_vel[i].AddPoint(t, myArray[i]);
				m_motor_status[i].act_velocity = myArray[i];
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
}
void UITrolleyControlTest::Thread_ReadActAcc()
{
	char szVar_acc[] = { "main.acc" };
	AdsExpand mmads(m_pAddr);
	while (!thread_read_acc_stop_flag)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		double* myArray = new double[m_trolley_num];
		mmads.ReadArray(szVar_acc, myArray, m_trolley_num);
		{
			std::lock_guard<std::mutex> lock(m_thread_read_acc_lock);
			for (unsigned int i = 0; i < m_trolley_num; i++)//��forѭ�������ʵ�ֶ�ȡ�����е�Ԫ��
			{
				float t = (float)glfwGetTime();
				m_vec_hist_data_acc[i].AddPoint(t, myArray[i]);
				m_motor_status[i].act_acceleration = myArray[i];
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
}

void UITrolleyControlTest::Thread_TrajectoryPositionToPlc(int axis_itx,std::vector<double> data)
{
	short itx = 0;
	char szVar_acc[256] = { };
	sprintf(szVar_acc, "AdsQueue.ArrayQueueBuffer_Len[%d]", axis_itx+1);
	char szVar_queue[256] = { };
	sprintf(szVar_queue,"AdsQueue.TrajectoryPosition_FromAds[%d]", axis_itx+1);
	char szVar_itx[256] = { };
	sprintf(szVar_itx, "AdsQueue.itx[%d]", axis_itx+1);
	AdsExpand mmads(m_pAddr);
	while (!(bool)thread_DoubleQueue_Write_stop_flag[axis_itx])
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		short* myArray = new short[1];
		double* myArray_ads = new double[1];
		long nErr = mmads.ReadArray(szVar_acc, myArray, 1);
		{
			if (myArray[0] < QueueBufferLen && !nErr && itx<data.size())
			{
				//std::lock_guard<std::mutex> lock(*m_thread_DoubleQueue_Write_lock[axis_itx]);
				myArray_ads[0] = data[itx];
				mmads.WriteArray(szVar_queue, myArray_ads, 1);
				itx++;
				mmads.WriteArray(szVar_itx, &itx, 1);
				UI_WARN(u8"axis:%d������λ��%d, ��%d�����ݵ�, ֵ��%lf", axis_itx + 1, myArray[0], itx, myArray_ads[0]);
			}

		}
		delete[] myArray;
		delete[] myArray_ads;
		auto endTime = std::chrono::high_resolution_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		int sleepTime = int(1000 / 200) - static_cast<int>(elapsedTime);
		if (sleepTime > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
		}
		if (itx== data.size())
		{
			thread_DoubleQueue_Write_stop_flag[axis_itx] = true;
			UI_WARN(u8"�켣�����߳�%d��ֹͣ", axis_itx + 1);
		}
	} 
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
	//for (int i = 0; i < m_ForceSensor_num; i++)
	//{
	//	if (m_slected_force_plot_index[i] > 0)
	//	{
	//		{
	//			std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
	//			if (!m_vec_hist_data_force[i].Data.empty())
	//			{
	//				tmp_count++;
	//				if (tmp_count == 1)
	//				{
	//					if (m_vec_hist_data_force[i].Data.size() < m_vec_hist_data_force[i].MaxSize) // �жϹ����������Ƿ���
	//					{
	//						os << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Data.size() - 1].x << ", " // ���ݵ�һ��Ҫ����ʱ���
	//							<< m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Data.size() - 1].y;
	//					}
	//					else
	//					{
	//						os << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Offset].x << ", "
	//							<< m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Offset].y;
	//					}
	//				}
	//				else
	//				{
	//					if (m_vec_hist_data_force[i].Data.size() < m_vec_hist_data_force[i].MaxSize)
	//					{
	//						os << "," << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Data.size() - 1].y;
	//					}
	//					else
	//					{
	//						os << ", " << m_vec_hist_data_force[i].Data[m_vec_hist_data_force[i].Offset].y;
	//					}
	//				}
	//			}
	//			else
	//			{
	//				UI_WARN(u8"��������%dû�����ݣ�", i);
	//				os << ", ";
	//			}
	//		}
	//	}
	//}
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

	//m_slected_force_plot_index.resize(m_ForceSensor_num);
	//m_vec_hist_data_force.resize(m_ForceSensor_num);
	//m_vec_hist_data_force_y_min.resize(m_ForceSensor_num, -1);
	//m_vec_hist_data_force_y_max.resize(m_ForceSensor_num, 1);
	//m_hist_data_force_y_min = -1;
	//m_hist_data_force_y_min = 1;

	Con_con_flag.resize(m_trolley_num, 0);
	PressDown.resize(m_trolley_num, 0);

	//m_thread_DoubleQueue_Write_lock.resize(m_trolley_num);
	thread_DoubleQueue_Write.resize(m_trolley_num);
	thread_DoubleQueue_Write_stop_flag.resize(m_trolley_num);
	//for (size_t i = 0; i < m_trolley_num; ++i) {
	//	m_thread_DoubleQueue_Write_lock.push_back(new std::mutex());
	//}

	m_TrajectoryFileFullPath.resize(m_trolley_num);
	m_TrajectoryDataAll.resize(m_trolley_num);
	

	m_record_flag_continuous = false;
	m_record_flag_jog = false;
	m_record_jog_num = 0;
	m_selected_plot_all = false;

	for (int j=0;j< m_trolley_num;++j)
	{
		m_motor_params[j].velocity_min = 0.01;
		m_motor_params[j].Tarvelocity = m_motor_params[j].velocity_min;
		m_TrajectoryFileFullPath[j] = new char[256]; // �����㹻���ڴ�
		std::strcpy(m_TrajectoryFileFullPath[j], "MotorTrajectory");
	}

	m_nPort = AdsPortOpenEx();		//��ADSͨѶ�˿�
	if (!ads_local_or_CX_flag)
	{
		long nErr = AdsGetLocalAddressEx(m_nPort, m_pAddr); //�Զ���ȡ���ص�ַ
		if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
	}
	m_pAddr->port = 851;			//TC3��ͨѶ�˿�Ϊ851
	AdsPortCloseEx(m_nPort);
	m_AdsExpand = new AdsExpand(m_pAddr);

	for (int i = 0; i < 10000; ++i)
	{
		test_array_ads[i] = sin(i*0.01)*18000;
	}

	
}

UITrolleyControlTest::~UITrolleyControlTest()
{
	if (m_AdsExpand)
	{
		delete m_AdsExpand;
		m_AdsExpand = nullptr;
	}
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

	if (thread_DoubleQueue_Write[0].joinable()) thread_DoubleQueue_Write[0].join();

	AdsPortCloseEx(m_nPort);

	for (int j = 0; j < m_trolley_max_his; ++j)
	{
		if (m_TrajectoryFileFullPath[j] != nullptr) {
			delete[] m_TrajectoryFileFullPath[j];
		}
		thread_DoubleQueue_Write_stop_flag[j] = 1;
	}
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

	//char sss[] = { "test" };
	//if (ImGui::Button(sss))
	//{
	//	{
	//		//std::lock_guard<std::mutex> lock(*m_thread_DoubleQueue_Write_lock[0]);
	//		thread_DoubleQueue_Write_stop_flag[0] = 0;
	//	}
	//	if (!thread_DoubleQueue_Write[0].joinable())
	//	{
	//		thread_DoubleQueue_Write[0] = std::thread(&UITrolleyControlTest::Thread_TrajectoryPositionToPlc, this,0);
	//	}
	//}


	// 0. �Ƿ�������ģ�飬��������ADS���̺߳�һ��ȫ��ADS�߳�
	char buf_module_enable[256] = { 0 };
	stbsp_sprintf(buf_module_enable, u8"%s ������ģ��", ICON_FA_MERCURY);
	bool tmp = ImGui::Checkbox(buf_module_enable, &Enable_this_module);
	if (tmp && Enable_this_module)
	{
		m_nPort = AdsPortOpenEx();		//��ADSͨѶ�˿�
		if (!ads_local_or_CX_flag)
		{
			long nErr = AdsGetLocalAddressEx(m_nPort, m_pAddr); //�Զ���ȡ���ص�ַ
			if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
		}
		m_pAddr->port = 851;			//TC3��ͨѶ�˿�Ϊ851
		AdsPortCloseEx(m_nPort);

		if (m_AdsExpand)
		{
			delete m_AdsExpand;
			m_AdsExpand = nullptr;
		}
		m_AdsExpand = new AdsExpand(m_pAddr);

		{
			std::lock_guard<std::mutex> lock(m_thread_read_pos_lock);
			thread_read_pos_stop_flag = false;
		}
		{
			std::lock_guard<std::mutex> lock(m_thread_read_vel_lock);
			thread_read_vel_stop_flag = false;
		}
		{
			std::lock_guard<std::mutex> lock(m_thread_read_acc_lock);
			thread_read_acc_stop_flag = false;
		}
		if (!thread_read_pos.joinable())
		{
			thread_read_pos = std::thread(&UITrolleyControlTest::Thread_ReadActPos, this);
		}
		if (!thread_read_vel.joinable())
		{
			thread_read_vel = std::thread(&UITrolleyControlTest::Thread_ReadActVel, this);
		}
		if (!thread_read_acc.joinable())
		{
			thread_read_acc = std::thread(&UITrolleyControlTest::Thread_ReadActAcc, this);
		}

		
	}
	if (Enable_this_module)
	{
		// ��ȡ���ʹ��״̬
		char szVar_EnableStatus[] = { "main.GetEnableStatus" };
		bool* myArray_Enable = new bool[m_trolley_num];
		long nErr = m_AdsExpand->ReadArray(szVar_EnableStatus, myArray_Enable, m_trolley_num);
		if (nErr) UI_ERROR("AdsError: %ld", nErr);
		else
		{
			for (int j = 0; j < m_trolley_num; j++)
			{
				m_motor_status[j].st_power = myArray_Enable[j];
				m_motor_params[j].set_enable = myArray_Enable[j];
			}
		}
		delete[] myArray_Enable;

		// ��ȡ�˶�״̬
		char szVar_MotionStatus[] = { "main.m_MotionState" };
		short* myArray_MotionStatus = new short[m_trolley_num];
		nErr = m_AdsExpand->ReadArray(szVar_MotionStatus, myArray_MotionStatus, m_trolley_num);
		if (nErr) UI_ERROR("AdsError: %ld", nErr);
		else
		{
			for (int j = 0; j < m_trolley_num; j++)
			{
				m_motor_status[j].st_motion_status = static_cast<MC_AxisStates>(myArray_MotionStatus[j]);
			}
		}
		delete[] myArray_MotionStatus;

		// ��ȡ�������״̬
		char szVar_ErrorStatus[] = { "main.GetErrorStatus" };
		bool* myArray_Error = new bool[m_trolley_num];
		nErr = m_AdsExpand->ReadArray(szVar_ErrorStatus, myArray_Error, m_trolley_num);
		if (nErr) UI_ERROR("AdsError: %ld", nErr);
		else
		{
			for (int j = 0; j < m_trolley_num; j++)
			{
				m_motor_status[j].st_error_status = myArray_Error[j];
				if (myArray_Error[j])
				{
					// ��ȡ�������ID
					char szVar_ErrorID[] = { "main.GetErrorId" };
					unsigned int* myArray_ErrorID = new unsigned int[m_trolley_num];
					nErr = m_AdsExpand->ReadArray(szVar_ErrorID, myArray_ErrorID, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					else
					{
						m_motor_status[j].st_status_error_id = myArray_ErrorID[j];
					}
					delete[] myArray_ErrorID;					
				}
				else
				{
					m_motor_status[j].st_status_error_id = 0;
				}
			}
		}
		delete[] myArray_Error;

		// ��ȡInTargetPosition״̬
		char szVar_InTargetPosition[] = { "main.GetInTargetPosition" };
		bool* myArray_InTargetPosition = new bool[m_trolley_num];
		nErr = m_AdsExpand->ReadArray(szVar_InTargetPosition, myArray_InTargetPosition, m_trolley_num);
		if (nErr) UI_ERROR("AdsError: %ld", nErr);
		else
		{
			for (int j = 0; j < m_trolley_num; j++)
			{
				m_motor_status[j].InTargetPosition = myArray_InTargetPosition[j];
			}
		}
		delete[] myArray_InTargetPosition;
	}
	if (tmp && !Enable_this_module)
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
	ImGui::Spacing();

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
					if (m_trolley_max_his < m_trolley_num_tmp)
					{
						m_TrajectoryFileFullPath.resize(m_trolley_num_tmp);
						for (int i = m_trolley_max_his; i < m_trolley_num_tmp; ++i)
						{
							m_TrajectoryFileFullPath[i] = new char[256]; // �����㹻���ڴ�
							std::strcpy(m_TrajectoryFileFullPath[i], "MotorTrajectory");
						}
						m_trolley_max_his = m_trolley_num_tmp;
						m_TrajectoryDataAll.resize(m_trolley_num_tmp);
					}
					
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

					Con_con_flag.resize(m_trolley_num, 0);
					PressDown.resize(m_trolley_num, 0);

					//m_thread_DoubleQueue_Write_lock.resize(m_trolley_num);
					thread_DoubleQueue_Write.resize(m_trolley_num);
					thread_DoubleQueue_Write_stop_flag.resize(m_trolley_num);

					

				}

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				//ImGui::SameLine();
				//if (ImGui::SliderInt("Num[1-16]", &m_ForceSensor_num_tmp, 1, m_ForceSensor_num_max)) // ͨ��slider���ñ��ص������
				//{

				//}
				//ImGui::SameLine();
				//stbsp_sprintf(buf, u8"%s ȷ�ϸ���������������", ICON_FA_MERCURY);
				//if (ImGui::Button(buf)) // ����������ťȷ����������ֹ�󻬶�
				//{
				//	m_ForceSensor_num = m_ForceSensor_num_tmp;
				//	m_slected_force_plot_index.resize(m_ForceSensor_num);
				//	m_vec_hist_data_force.resize(m_ForceSensor_num);
				//	m_vec_hist_data_force_y_min.resize(m_ForceSensor_num, -1);
				//	m_vec_hist_data_force_y_max.resize(m_ForceSensor_num, 1);
				//}
			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s һ�������˶�ģʽ", ICON_FA_LIST);
			if (ImGui::Button(buf)) // �����˶�ģʽѡ��ť
			{
				// �л�����ģʽǰ��ֹͣ
				unsigned long lHdlVar;   	//�������-ʹ��
				unsigned long pcbReturn;
				char szVar_Motion_Exc[] = { "main.m_Motion_Exc" };
				char szVar_stop[] = { "main.SetStop" };
				char szVar[] = { "main.m_SelectedMotionMode" };

				bool* myArray_Motion_Exc = new bool[m_trolley_num];
				bool* myArray_stop = new bool[m_trolley_num];
				short* myArray = new short[m_trolley_num];
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_selected_motion_mode[i] = m_selected_motion_mode_ALL;
						m_motor_params[i].motion_exc = false;
						m_motor_params[i].set_stop = true;
						m_motor_params[i].selected_motion_mode = m_selected_motion_mode[i];
					}
					myArray_Motion_Exc[i] = m_motor_params[i].motion_exc;
					myArray_stop[i] = m_motor_params[i].set_stop;
					myArray[i] = m_motor_params[i].selected_motion_mode;
				}

				long nErr = m_AdsExpand->WriteArray(szVar_Motion_Exc, myArray_Motion_Exc, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);

				delete[] myArray_Motion_Exc;
				delete[] myArray_stop;
				delete[] myArray;
			}

			char preview_value[64] = { 0 };
			if (m_selected_motion_mode_ALL >= 0)
			{
				snprintf(preview_value, sizeof(preview_value), "%s %s", ICON_FA_MICROCHIP, MotorModeNames[m_selected_motion_mode_ALL]);
			}
			ImGui::SameLine();
			if (ImGui::BeginCombo(u8"�˶�ģʽ�б�", preview_value, 0)) // �����˶�ģʽ�����б�
			{
				for (int i = 0; i < motorMotionModeCount; i++)
				{
					stbsp_sprintf(buf, u8"%s %s", ICON_FA_MICROCHIP, MotorModeNames[i]);
					if (ImGui::Selectable(buf, m_selected_motion_mode_ALL == i))
					{
						m_selected_motion_mode_ALL = i;
						//for (int j = 0; j < m_trolley_num; ++j)
						//{
						//	if (m_selected_flag[j])
						//	{
						//		m_selected_motion_mode[j] = m_selected_motion_mode_ALL;
						//	}
						//}
					}
				}
				ImGui::EndCombo();
			}

			stbsp_sprintf(buf, u8"%s һ��ʹ��", ICON_FA_EYE);
			if (ImGui::Button(buf)) // ������ע�ᵽ������Ƶĵ��ʹ��
			{
				unsigned long lHdlVar;   	//�������-ʹ��
				unsigned long pcbReturn;
				char szVar[] = { "main.SetEnable" };
				bool* myArray = new bool[m_trolley_num];
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_motor_params[i].set_enable = true;
					}
					myArray[i] = m_motor_params[i].set_enable;
				}
				long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				delete[] myArray;
			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s һ���Ͽ�ʹ��", ICON_FA_EYE_DROPPER); // ������ע�ᵽ������Ƶĵ���Ͽ�ʹ��
			if (ImGui::Button(buf))
			{
				unsigned long lHdlVar;   	//�������-ʹ��
				unsigned long pcbReturn;
				char szVar[] = { "main.SetEnable" };
				bool* myArray = new bool[m_trolley_num];
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_motor_params[i].set_enable = false;
					}
					myArray[i] = m_motor_params[i].set_enable;
				}
				long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				delete[] myArray;
			}

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
			stbsp_sprintf(buf, u8"%s һ����ͣ", ICON_FA_STOP); // ������ע�ᵽ������Ƶĵ����ͣ
			if (ImGui::Button(buf))
			{
				unsigned long lHdlVar;   	//�������-ʹ��
				unsigned long pcbReturn;
				char szVar_exc[] = { "main.m_Motion_Exc" };
				char szVar_halt[] = { "main.SetHalt" };
				bool* myArray_motion_exc = new bool[m_trolley_num];
				bool* myArray = new bool[m_trolley_num];
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_motor_params[i].set_halt = true;
						m_motor_params[i].set_continue = false;
						m_motor_params[i].motion_exc = false;
					}
					myArray_motion_exc[i] = m_motor_params[i].motion_exc;
					myArray[i] = m_motor_params[i].set_halt;
				}
				long nErr = m_AdsExpand->WriteArray(szVar_exc, myArray_motion_exc, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				nErr = m_AdsExpand->WriteArray(szVar_halt, myArray, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				delete[] myArray_motion_exc;
				delete[] myArray;
			}
			ImGui::PopStyleColor(4);
			
			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s һ������", ICON_FA_COG); // ������ע�ᵽ������Ƶĵ������
			if (ImGui::Button(buf))
			{
				unsigned long lHdlVar;   	//�������-ʹ��
				unsigned long pcbReturn;
				char szVar[] = { "main.SetContinue" };
				char szVar_motion_exc[] = { "main.m_Motion_Exc" };
				bool* myArray = new bool[m_trolley_num];
				bool* myArray_motion_exc = new bool[m_trolley_num];
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_motor_params[i].set_continue = true;
						m_motor_params[i].set_halt = false;
						m_motor_params[i].motion_exc = true;
					}
					myArray[i] = m_motor_params[i].set_continue;
					myArray_motion_exc[i] = m_motor_params[i].motion_exc;
					m_motor_params[i].set_continue = false;
				}
				long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				nErr = m_AdsExpand->WriteArray(szVar_motion_exc, myArray_motion_exc, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				delete[] myArray;
				delete[] myArray_motion_exc;
			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s һ���������", ICON_FA_GLASSES); // ������ע�ᵽ������Ƶĵ���������
			if (ImGui::Button(buf))
			{
				unsigned long lHdlVar;   	//�������-ʹ��
				unsigned long pcbReturn;
				char szVar[] = { "main.SetClear" };
				bool* myArray = new bool[m_trolley_num];
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_motor_params[i].set_clear = true;
					}
					myArray[i] = m_motor_params[i].set_clear;
					m_motor_params[i].set_clear = false;
				}
				long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
				if (nErr) UI_ERROR("AdsError: %ld", nErr);
				delete[] myArray;
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
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, u8"һ������ȷ��");
				if (ImGui::Button(buf))
				{

				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"�㶯��������", ImGuiDataType_U32, (void*)&m_motor_params_ALL.jog_velocity_fast);

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
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
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar[] = { "main.m_Move_Vel" };
					
					double* myArray = new double[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].Tarvelocity = m_motor_params_ALL.Tarvelocity;
						}
						myArray[i] = m_motor_params[i].Tarvelocity;
					}
					long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_Double, (void*)&m_motor_params_ALL.Tarvelocity);

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (PressDown_All) ? (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f) : (ImVec4)ImColor::HSV(150.0f / 360.0f, 0.9f, 0.9f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ����ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
				if (ImGui::Button(buf))
				{
					PressDown_All = true;
					// ��ʼ�˶���ť�������clear�����ܴ��ڵ�stop״̬
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar_stop[] = { "main.SetStop" };
					bool* myArray_stop = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].set_stop = false;
							PressDown[i] = 1;
						}
						myArray_stop[i] = m_motor_params[i].set_stop;
					}
					long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_stop;

					char szVar_clear[] = { "main.SetClear" };
					bool* myArray_clear = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].set_clear = true;
						}
						myArray_clear[i] = m_motor_params[i].set_clear;
					}
					nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_clear;

					char szVar[] = { "main.m_Motion_Exc" };
					bool* myArray = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].motion_exc = true;
						}
						myArray[i] = m_motor_params[i].motion_exc;
					}
					nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
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
					PressDown_All = false;
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar[] = { "main.m_Motion_Exc" };
					char szVar_stop[] = { "main.SetStop" };
					bool* myArray = new bool[m_trolley_num];
					bool* myArray_stop = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							PressDown[i] = 0;
							m_motor_params[i].motion_exc = false;
							m_motor_params[i].set_stop = true;
						}
						myArray[i] = m_motor_params[i].motion_exc;
						myArray_stop[i] = m_motor_params[i].set_stop;
					}
					long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
					delete[] myArray_stop;
				}
				ImGui::PopStyleColor(4);
				break;
			case Position_mode:
				stbsp_sprintf(buf, u8"%s һ������λ��", ICON_FA_GRIN_STARS);
				if (ImGui::Button(buf))
				{
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].Tarposition = m_motor_params_ALL.Tarposition;
						}
					}
				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_Double, (void*)&m_motor_params_ALL.Tarposition);

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s һ�������ٶ�", ICON_FA_CAR);
				if (ImGui::Button(buf))
				{
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar[] = { "main.m_Move_Vel" };

					double* myArray = new double[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].Tarvelocity = m_motor_params_ALL.Tarvelocity;
						}
						myArray[i] = m_motor_params[i].Tarvelocity;
					}
					long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_Double, (void*)&m_motor_params_ALL.Tarvelocity);

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ����ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
				if (ImGui::Button(buf))
				{
					// ��ʼ�˶���ť�������clear�����ܴ��ڵ�stop״̬
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar_stop[] = { "main.SetStop" };
					bool* myArray_stop = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							PressDown[i] = 1;
							m_motor_params[i].set_stop = false;
						}
						myArray_stop[i] = m_motor_params[i].set_stop;
					}
					long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_stop;

					char szVar_clear[] = { "main.SetClear" };
					bool* myArray_clear = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].set_clear = true;
						}
						myArray_clear[i] = m_motor_params[i].set_clear;
					}
					nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_clear;

					char szVar_position[] = { "main.m_Move_Pos" };
					double* myArray_position = new double[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						myArray_position[i] = m_motor_params[i].Tarposition;
					}
					nErr = m_AdsExpand->WriteArray(szVar_position, myArray_position, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_position;

					char szVar[] = { "main.m_Motion_Exc" };
					bool* myArray = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].motion_exc = true;
						}
						myArray[i] = m_motor_params[i].motion_exc;
					}
					nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
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
					PressDown_All = false;
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar[] = { "main.m_Motion_Exc" };
					char szVar_stop[] = { "main.SetStop" };
					bool* myArray = new bool[m_trolley_num];
					bool* myArray_stop = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							PressDown[i] = 0;
							m_motor_params[i].motion_exc = false;
							m_motor_params[i].set_stop = true;
						}
						myArray[i] = m_motor_params[i].motion_exc;
						myArray_stop[i] = m_motor_params[i].set_stop;
					}
					long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
					delete[] myArray_stop;
				}
				ImGui::PopStyleColor(4);
				break;
			case Continus_mode:
				stbsp_sprintf(buf, u8"%s һ�������ٶ�����", ICON_FA_ROCKET);
				if (ImGui::Button(buf))
				{
					if (m_motor_params_ALL.Tarvelocity > m_motor_params_ALL.velocity_min)
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Vel" };
						
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							if (m_selected_flag[i])
							{
								m_motor_params[i].Tarvelocity = m_motor_params_ALL.Tarvelocity;
							}
							myArray[i] = m_motor_params[i].Tarvelocity;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}
					else
					{
						UI_ERROR("Error: Velocity is too small: %lf", m_motor_params_ALL.Tarvelocity);
						m_motor_params_ALL.Tarvelocity = m_motor_params_ALL.velocity_min;
					}
				}

				ImGui::SameLine();
				ImGui::InputScalar(u8"�ٶ�����", ImGuiDataType_Double, (void*)&m_motor_params_ALL.Tarvelocity);

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_Double, (void*)&m_motor_params_ALL.Tarposition, NULL, NULL, "%f", ImGuiInputTextFlags_EnterReturnsTrue);
				for (int i = 0; i < m_trolley_num; ++i)
				{
					if (m_selected_flag[i])
					{
						m_motor_params[i].Tarposition = m_motor_params_ALL.Tarposition;
					}
				}

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (PressDown_All) ? (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f) : (ImVec4)ImColor::HSV(150.0f / 360.0f, 0.9f, 0.9f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ����ʼ�˶�", ICON_FA_RUNNING);
				if (ImGui::Button(buf))
				{
					PressDown_All = true;
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar_stop[] = { "main.SetStop" };
					char szVar_clear[] = { "main.SetClear" };
					char szVar[] = { "main.m_Motion_Exc" };
					bool* myArray_stop = new bool[m_trolley_num];
					bool* myArray_clear = new bool[m_trolley_num];
					bool* myArray = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							PressDown[i] = 1;
							m_motor_params[i].set_stop = false;
							m_motor_params[i].set_clear = true;
							Con_con_flag[i] = 1;
							m_motor_params[i].motion_exc = true;
						}
						myArray_stop[i] = m_motor_params[i].set_stop;
						myArray_clear[i] = m_motor_params[i].set_clear;
						myArray[i] = m_motor_params[i].motion_exc;
					}
					long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_clear;
					delete[] myArray_stop;
					delete[] myArray;
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
					PressDown_All = false;
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar[] = { "main.m_Motion_Exc" };
					char szVar_stop[] = { "main.SetStop" };
					bool* myArray = new bool[m_trolley_num];
					bool* myArray_stop = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							PressDown[i] = 0;
							Con_con_flag[i] = 0;
							m_motor_params[i].motion_exc = false;
							m_motor_params[i].set_stop = true;
						}
						myArray[i] = m_motor_params[i].motion_exc;
						myArray_stop[i] = m_motor_params[i].set_stop;
					}
					long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
					delete[] myArray_stop;
				}
				ImGui::PopStyleColor(4);

				{
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar[] = { "main.m_Move_Pos" };
					double* myArray = new double[m_trolley_num];
					for (int j = 0; j < m_trolley_num; ++j)
					{
						if (Con_con_flag[j])
						{
							myArray[j] = m_motor_params[j].Tarposition;
						}
					}
					long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
				}

				break;
			case Trajectory_mode:
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
				stbsp_sprintf(buf, u8"%s һ����ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
				if (ImGui::Button(buf))
				{
					// ��ʼ�˶���ť�������clear�����ܴ��ڵ�stop״̬
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar_stop[] = { "main.SetStop" };
					bool* myArray_stop = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							PressDown[i] = 1;
							m_motor_params[i].set_stop = false;
						}
						myArray_stop[i] = m_motor_params[i].set_stop;
					}
					long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_stop;

					char szVar_clear[] = { "main.SetClear" };
					bool* myArray_clear = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].set_clear = true;
						}
						myArray_clear[i] = m_motor_params[i].set_clear;
					}
					nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray_clear;

					char szVar[] = { "main.m_Motion_Exc" };
					bool* myArray = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							m_motor_params[i].motion_exc = true;
						}
						myArray[i] = m_motor_params[i].motion_exc;
					}
					nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;

					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							if (!thread_DoubleQueue_Write[i].joinable())
							{
								thread_DoubleQueue_Write_stop_flag[i] = 0;
								thread_DoubleQueue_Write[i] = std::thread(&UITrolleyControlTest::Thread_TrajectoryPositionToPlc, this, i, m_TrajectoryDataAll[i]);
								thread_DoubleQueue_Write[i].detach();
							}
						}
					}
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
					PressDown_All = false;
					unsigned long lHdlVar;   	//�������-ʹ��
					unsigned long pcbReturn;
					char szVar[] = { "main.m_Motion_Exc" };
					char szVar_stop[] = { "main.SetStop" };
					bool* myArray = new bool[m_trolley_num];
					bool* myArray_stop = new bool[m_trolley_num];
					for (int i = 0; i < m_trolley_num; ++i)
					{
						if (m_selected_flag[i])
						{
							PressDown[i] = 0;
							m_motor_params[i].motion_exc = false;
							m_motor_params[i].set_stop = true;
						}
						myArray[i] = m_motor_params[i].motion_exc;
						myArray_stop[i] = m_motor_params[i].set_stop;
					}
					long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
					if (nErr) UI_ERROR("AdsError: %ld", nErr);
					delete[] myArray;
					delete[] myArray_stop;
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
				stbsp_sprintf(buf, "  |  ");
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
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
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
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ͣ", ICON_FA_STOP);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar_exc[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = false;
						bool* myArray_motion_exc = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_motion_exc[i] = m_motor_params[i].motion_exc;
						}
						long nErr = m_AdsExpand->WriteArray(szVar_exc, myArray_motion_exc, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_motion_exc;

						char szVar_halt[] = { "main.SetHalt" };
						m_motor_params[j].set_halt = true;
						m_motor_params[j].set_continue = false;
						m_motor_params[j].motion_exc = false;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].set_halt;
						}
						nErr = m_AdsExpand->WriteArray(szVar_halt, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
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
						m_motor_params[j].set_halt = false;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].set_continue;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
						m_motor_params[j].set_continue = false;

						char szVar_motion_exc[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = true;
						bool* myArray_motion_exc = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_motion_exc[i] = m_motor_params[i].motion_exc;
						}
						nErr = m_AdsExpand->WriteArray(szVar_motion_exc, myArray_motion_exc, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_motion_exc;
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
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}
					else
					{
						m_motor_params[j].set_clear = false;
					}
				}
				//ImGui::EndGroup();

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
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
						for (int i = 0; i < motorMotionModeCount; i++)
						{
							stbsp_sprintf(buf, u8"%s %s", ICON_FA_MICROCHIP, MotorModeNames[i]);
							if (ImGui::Selectable(buf, m_selected_motion_mode[j] == i)) // �б��е�ѡ����¼��
							{	
								m_selected_motion_mode_last[j] = m_selected_motion_mode[j];
								m_selected_motion_mode[j] = i;
								if (m_selected_motion_mode[j] != m_selected_motion_mode_last[j])
								{
									// �л�����ģʽǰ��ֹͣ
									unsigned long lHdlVar;   	//�������-ʹ��
									unsigned long pcbReturn;
									char szVar_Motion_Exc[] = { "main.m_Motion_Exc" };
									m_motor_params[j].motion_exc = false;
									bool* myArray_Motion_Exc = new bool[m_trolley_num];
									for (int i = 0; i < m_trolley_num; ++i)
									{
										myArray_Motion_Exc[i] = m_motor_params[i].motion_exc;
									}
									long nErr = m_AdsExpand->WriteArray(szVar_Motion_Exc, myArray_Motion_Exc, m_trolley_num);
									if (nErr) UI_ERROR("AdsError: %ld", nErr);
									delete[] myArray_Motion_Exc;

									char szVar_stop[] = { "main.SetStop" };
									m_motor_params[j].set_stop = true;
									bool* myArray_stop = new bool[m_trolley_num];
									for (int i = 0; i < m_trolley_num; ++i)
									{
										myArray_stop[i] = m_motor_params[i].set_stop;
									}
									nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
									if (nErr) UI_ERROR("AdsError: %ld", nErr);
									delete[] myArray_stop;

									char szVar[] = { "main.m_SelectedMotionMode" };
									m_motor_params[j].selected_motion_mode = m_selected_motion_mode[j];
									short* myArray = new short[m_trolley_num];
									for (int i = 0; i < m_trolley_num; ++i)
									{
										myArray[i] = m_motor_params[i].selected_motion_mode;
									}
									nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
									if (nErr) UI_ERROR("AdsError: %ld", nErr);
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
					stbsp_sprintf(buf, "  |  ");
					ImGui::Text(buf);

					ImGui::SameLine();
					stbsp_sprintf(buf, u8"����ȷ��");
					if (ImGui::Button(buf))
					{

					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"�㶯��������", ImGuiDataType_U32, (void*)&m_motor_params[j].jog_velocity_fast);

					ImGui::SameLine();
					stbsp_sprintf(buf, "  |  ");
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
						m_motor_params[j].Tarvelocity = m_motor_params[j].Tarvelocity;
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].Tarvelocity;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_Double, (void*)&m_motor_params[j].Tarvelocity);

					ImGui::SameLine();
					stbsp_sprintf(buf, "  |  ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, ((bool)PressDown[j]) ? (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f) : (ImVec4)ImColor::HSV(150.0f / 360.0f, 0.9f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
					if (ImGui::Button(buf))
					{
						PressDown[j] = 1;
						// ��ʼ�˶���ť�������clear�����ܴ��ڵ�stop״̬
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = false;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;

						char szVar_clear[] = { "main.SetClear" };
						m_motor_params[j].set_clear = true;
						bool* myArray_clear = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_clear[i] = m_motor_params[i].set_clear;
						}
						nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_clear;

						char szVar[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].motion_exc;
						}
						nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}
					ImGui::PopStyleColor(4);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(10.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(20.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ֹͣ�˶�", ICON_FA_SHARE);
					if (ImGui::Button(buf))
					{
						PressDown[j] = false;

						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = false;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].motion_exc;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;

						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = true;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;
					}
					ImGui::PopStyleColor(4);
					break;
				case Position_mode://λ��ģʽ������ģʽ�������ٶȻ��ǵ÷ֿ�
					stbsp_sprintf(buf, u8"%s ����λ��", ICON_FA_GRIN_STARS);
					if (ImGui::Button(buf))
					{
						m_motor_params[j].Tarposition = m_motor_params[j].Tarposition;
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_Double, (void*)&m_motor_params[j].Tarposition);

					ImGui::SameLine();
					stbsp_sprintf(buf, "  |  ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::SameLine();
					stbsp_sprintf(buf, u8"%s �����ٶ�", ICON_FA_CAR);
					if (ImGui::Button(buf))
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Vel" };
						m_motor_params[j].Tarvelocity = m_motor_params[j].Tarvelocity;
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].Tarvelocity;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"Ŀ���ٶ�", ImGuiDataType_Double, (void*)&m_motor_params[j].Tarvelocity);

					ImGui::SameLine();
					stbsp_sprintf(buf, "  |  ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, ((bool)PressDown[j]) ? (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f) : (ImVec4)ImColor::HSV(150.0f / 360.0f, 0.9f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ʼ�˶�", ICON_FA_LONG_ARROW_ALT_UP);
					if (ImGui::Button(buf))
					{
						// ��ʼ�˶���ť�������clear�����ܴ��ڵ�stop״̬
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = false;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;

						char szVar_clear[] = { "main.SetClear" };
						m_motor_params[j].set_clear = true;
						bool* myArray_clear = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_clear[i] = m_motor_params[i].set_clear;
						}
						nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_clear;

						char szVar[] = { "main.m_Move_Pos" };
						m_motor_params[j].Tarposition = m_motor_params[j].Tarposition;
						double* myArray_position = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_position[i] = m_motor_params[i].Tarposition;
						}
						nErr = m_AdsExpand->WriteArray(szVar, myArray_position, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_position;

						if (m_motor_params[j].Tarvelocity > m_motor_params[j].velocity_min)
						{
							unsigned long lHdlVar;   	//�������-ʹ��
							unsigned long pcbReturn;
							char szVar[] = { "main.m_Motion_Exc" };
							m_motor_params[j].motion_exc = true;
							bool* myArray = new bool[m_trolley_num];
							for (int i = 0; i < m_trolley_num; ++i)
							{
								myArray[i] = m_motor_params[i].motion_exc;
							}
							long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
							if (nErr) UI_ERROR("AdsError: %ld", nErr);
							delete[] myArray;
						}
						else
						{
							UI_ERROR("Error: Velocity is too small: %lf", m_motor_params[j].Tarvelocity);
						}
					}
					ImGui::PopStyleColor(4);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(10.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(20.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ֹͣ�˶�", ICON_FA_SHARE);
					if (ImGui::Button(buf))
					{
						PressDown[j] = false;

						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = false;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].motion_exc;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;

						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = true;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;
					}
					ImGui::PopStyleColor(4);
					break;
				case Continus_mode:
					stbsp_sprintf(buf, u8"%s �����ٶ�����", ICON_FA_ROCKET);
					if (ImGui::Button(buf))
					{
						if (m_motor_params[j].Tarvelocity > m_motor_params[j].velocity_min)
						{
							unsigned long lHdlVar;   	//�������-ʹ��
							unsigned long pcbReturn;
							char szVar[] = { "main.m_Move_Vel" };
							m_motor_params[j].Tarvelocity = m_motor_params[j].Tarvelocity;
							double* myArray = new double[m_trolley_num];
							for (int i = 0; i < m_trolley_num; ++i)
							{
								myArray[i] = m_motor_params[i].Tarvelocity;
							}
							long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
							if (nErr) UI_ERROR("AdsError: %ld", nErr);
							delete[] myArray;
						}
						else
						{
							UI_ERROR("Error: Velocity is too small: %lf", m_motor_params[j].Tarvelocity);
							m_motor_params[j].Tarvelocity = m_motor_params[j].velocity_min;
						}
					}

					ImGui::SameLine();
					ImGui::InputScalar(u8"�ٶ�����", ImGuiDataType_Double, (void*)&m_motor_params[j].Tarvelocity);

					ImGui::SameLine();
					stbsp_sprintf(buf, "  |  ");
					ImGui::Text(buf);

					ImGui::SameLine();
					// ImGuiInputTextFlags_EnterReturnsTrue ʹ�������־����������Ҫ��һ�»س����ܸ�������
					ImGui::InputScalar(u8"Ŀ��λ��", ImGuiDataType_Double, (void*)&m_motor_params[j].Tarposition, NULL, NULL, "%f", ImGuiInputTextFlags_EnterReturnsTrue);

					ImGui::SameLine();
					stbsp_sprintf(buf, "  |  ");
					ImGui::Text(buf);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button,((bool)PressDown[j])? (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f): (ImVec4)ImColor::HSV(150.0f / 360.0f, 0.9f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ʼ�˶�", ICON_FA_RUNNING);
					if (ImGui::Button(buf))
					{
						PressDown[j] = true;
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = false;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;

						char szVar_clear[] = { "main.SetClear" };
						m_motor_params[j].set_clear = true;
						bool* myArray_clear = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_clear[i] = m_motor_params[i].set_clear;
						}
						nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_clear;

						Con_con_flag[j] = 1;

						char szVar[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].motion_exc;
						}
						nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}
					ImGui::PopStyleColor(4);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(10.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(20.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ֹͣ�˶�", ICON_FA_SHARE);
					if (ImGui::Button(buf))
					{
						PressDown[j] = false;
						Con_con_flag[j] = 0;

						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = false;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].motion_exc;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;

						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = true;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;
					}

					if (Con_con_flag[j])
					{
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Move_Pos" };
						double* myArray = new double[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].Tarposition;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;
					}

					ImGui::PopStyleColor(4);
					break;
				case Trajectory_mode:
					stbsp_sprintf(buf, u8"%s ��ȡ�켣�ļ�", ICON_FA_ROCKET);
					if (ImGui::Button(buf))
					{
						m_TrajectoryDataAll[j].clear();
						m_TrajectoryDataAll[j].resize(0);
						std::ifstream infile("data.csv", std::ifstream::in);
						if (!infile.is_open())
						{
							LOG_ERROR("Load data failed, file %s does not exists!\n", m_TrajectoryFileFullPath[j]);
						}
						else
						{
							int count = 0;
							while (!infile.eof())
							{
								std::string s;
								std::getline(infile, s);
								if (s.empty())
								{
									continue;
								}
								double tmp;
								std::istringstream iss(s);
								iss >> tmp;
								m_TrajectoryDataAll[j].push_back(tmp);
								count++;
							}
							infile.close();
						}
					}

					ImGui::SameLine();
					ImGui::PushItemWidth(0.7 * (ImGui::GetWindowWidth() - 200.0f));
					if (ImGui::InputText(u8"�켣�ļ�·��", m_TrajectoryFileFullPath[j], 256))
					{
						//UI_ERROR(m_TrajectoryFileFullPath[j]);
					}
					ImGui::PopItemWidth();
					

					//ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, ((bool)PressDown[j]) ? (ImVec4)ImColor::HSV(100.0f / 360.0f, 0.5f, 0.5f) : (ImVec4)ImColor::HSV(150.0f / 360.0f, 0.9f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(110.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(120.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ��ʼ�˶�", ICON_FA_RUNNING);
					if (ImGui::Button(buf))
					{
						PressDown[j] = true;
						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = false;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						long nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;

						char szVar_clear[] = { "main.SetClear" };
						m_motor_params[j].set_clear = true;
						bool* myArray_clear = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_clear[i] = m_motor_params[i].set_clear;
						}
						nErr = m_AdsExpand->WriteArray(szVar_clear, myArray_clear, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_clear;

						char szVar[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = true;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].motion_exc;
						}
						nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;

						if (!thread_DoubleQueue_Write[j].joinable())
						{
							thread_DoubleQueue_Write_stop_flag[j] = 0;
							thread_DoubleQueue_Write[j] = std::thread(&UITrolleyControlTest::Thread_TrajectoryPositionToPlc, this, j, m_TrajectoryDataAll[j]);
							thread_DoubleQueue_Write[j].detach();
						}
					}
					ImGui::PopStyleColor(4);

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f / 360.0f, 0.5f, 0.5f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(10.0f / 360.0f, 0.7f, 0.7f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(20.0f / 360.0f, 0.8f, 0.8f));
					stbsp_sprintf(buf, u8"%s ֹͣ�˶�", ICON_FA_SHARE);
					if (ImGui::Button(buf))
					{
						PressDown[j] = false;

						unsigned long lHdlVar;   	//�������-ʹ��
						unsigned long pcbReturn;
						char szVar[] = { "main.m_Motion_Exc" };
						m_motor_params[j].motion_exc = false;
						bool* myArray = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray[i] = m_motor_params[i].motion_exc;
						}
						long nErr = m_AdsExpand->WriteArray(szVar, myArray, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray;

						char szVar_stop[] = { "main.SetStop" };
						m_motor_params[j].set_stop = true;
						bool* myArray_stop = new bool[m_trolley_num];
						for (int i = 0; i < m_trolley_num; ++i)
						{
							myArray_stop[i] = m_motor_params[i].set_stop;
						}
						nErr = m_AdsExpand->WriteArray(szVar_stop, myArray_stop, m_trolley_num);
						if (nErr) UI_ERROR("AdsError: %ld", nErr);
						delete[] myArray_stop;
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
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s ���״̬:", ICON_FA_PASTAFARIANISM);
				ImGui::Text(buf);

				ImGui::SameLine();
				switch (m_motor_status[j].st_error_status)
				{
				case 0:
					stbsp_sprintf(buf, u8"����");
					ImGui::Text(buf);
					break;
				case 1:
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
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s �˶�״̬:", ICON_FA_PASTAFARIANISM);
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, MC_AxisStateToString(m_motor_status[j].st_motion_status).c_str());
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				stbsp_sprintf(buf, u8"%s ִ��״̬:", ICON_FA_PASTAFARIANISM);
				ImGui::Text(buf);

				ImGui::SameLine();
				switch (m_motor_status[j].InTargetPosition)
				{
				case 0:
					stbsp_sprintf(buf, u8"Ŀ��ƫ��");
					ImGui::Text(buf);
					break;
				case 1:
					stbsp_sprintf(buf, u8"�˶���λ");
					ImGui::Text(buf);
					break;
				default:
					stbsp_sprintf(buf, u8"δ֪");
					ImGui::Text(buf);
					break;
				}

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::PushItemWidth(0.06f * ImGui::GetWindowWidth());
				ImGui::SameLine();
				// ʹ��ImGuiInputTextFlags_ReadOnly��ʹ�ı����Ϊֻ��
				ImGui::InputScalar(u8"��ǰλ��", ImGuiDataType_Double, &m_motor_status[j].act_position, NULL, NULL, "%f", ImGuiInputTextFlags_ReadOnly);
				ImGui::SameLine();

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::InputScalar(u8"��ǰ�ٶ�", ImGuiDataType_Double, &m_motor_status[j].act_velocity, NULL, NULL, "%f", ImGuiInputTextFlags_ReadOnly);
				ImGui::SameLine();

				ImGui::SameLine();
				stbsp_sprintf(buf, "  |  ");
				ImGui::Text(buf);

				ImGui::SameLine();
				ImGui::InputScalar(u8"��ǰ���ٶ�", ImGuiDataType_Double, &m_motor_status[j].act_acceleration, NULL, NULL, "%f", ImGuiInputTextFlags_ReadOnly);
				ImGui::PopItemWidth();

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

	if (ImGui::CollapsingHeader(u8"С����Ϣ��ͼ�ͼ�¼", ImGuiTreeNodeFlags_DefaultOpen))
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

		//stbsp_sprintf(buf, u8"%s ѡ����������ID", ICON_FA_LIST);
		//ImGui::Text(buf);

		// ѡ�����е�������������ͼ
		//ImGui::SameLine();
		ImGui::PushID(1000);
		stbsp_sprintf(buf, u8"ȫѡ");
		if (m_record_flag_continuous || m_record_flag_jog)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		//bool tmp2 = ImGui::Checkbox(buf, &m_selected_force_plot_all);
		//if (tmp2 && m_selected_force_plot_all)
		//{
		//	for (int i = 0; i < m_ForceSensor_num; ++i)
		//	{
		//		m_slected_force_plot_index[i] = 1;
		//	}
		//}
		//if (tmp2 && !m_selected_force_plot_all)
		//{
		//	for (int i = 0; i < m_ForceSensor_num; ++i)
		//	{
		//		m_slected_force_plot_index[i] = 0;
		//	}
		//}
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
		//for (int i = 0; i < m_ForceSensor_num; ++i) {
		//	ImGui::CheckboxFlags((u8"�������� " + std::to_string(i + 1)).c_str(), &m_slected_force_plot_index[i], 1);
		//	ImGui::NextColumn();
		//}
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
				//for (int i = 0; i < m_ForceSensor_num; i++)
				//{
				//	if (m_slected_force_plot_index[i] > 0) // ������ѡ�����Ҫ��ͼ�������������������ͷ
				//	{
				//		os << "," << u8"��������" + std::to_string(i + 1);
				//	}
				//}
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
					//for (int i = 0; i < m_ForceSensor_num; i++)
					//{
					//	if (m_slected_force_plot_index[i] > 0) // ������ѡ�����Ҫ��ͼ�������������������ͷ
					//	{
					//		os << "," << u8"��������" + std::to_string(i + 1);
					//	}
					//}
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
		//{
		//	std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
		//	for (unsigned int i = 0; i < m_ForceSensor_num; i++)
		//	{
		//		float t = (float)glfwGetTime();
		//		m_vec_hist_data_force[i].AddPoint(t, std::sin(t));
		//	}
		//}


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

		//if (ImPlot::BeginPlot(u8"������������", ImVec2(-1, 350)))
		//{
		//	{
		//		// ��������������е������Сֵ������axis limit����
		//		std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
		//		m_hist_data_force_y_min = FLT_MAX;
		//		m_hist_data_force_y_max = -FLT_MAX;
		//		for (unsigned int i = 0; i < m_ForceSensor_num; i++)
		//		{
		//			if (!m_vec_hist_data_force[i].Data.empty()) // �жϹ����������Ƿ���
		//			{
		//				float max_y_value = std::max_element(m_vec_hist_data_force[i].Data.begin(), m_vec_hist_data_force[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
		//					return a.y < b.y;
		//					})->y;
		//				float min_y_value = std::min_element(m_vec_hist_data_force[i].Data.begin(), m_vec_hist_data_force[i].Data.end(), [](const ImVec2& a, const ImVec2& b) {
		//					return a.y < b.y;
		//					})->y;
		//				m_vec_hist_data_force_y_max[i] = max_y_value;
		//				m_vec_hist_data_force_y_min[i] = min_y_value;

		//				if (m_slected_force_plot_index[i])
		//				{
		//					m_hist_data_force_y_min = min(m_vec_hist_data_force_y_min[i], m_hist_data_force_y_min);
		//					m_hist_data_force_y_max = max(m_vec_hist_data_force_y_max[i], m_hist_data_force_y_max);
		//				}
		//			}
		//		}
		//	}

		//	ImPlot::SetupAxes("t(s)", "ForceSensor", flags, flags);
		//	ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always); // ����x�����ʱ�����
		//	// ���ݼ�ֵ����y��limit
		//	if (m_hist_data_force_y_max == m_hist_data_force_y_min)
		//	{
		//		ImGui::PushID(105);
		//		ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_force_y_min - 1, m_hist_data_force_y_max + 1, ImGuiCond_Always);
		//		ImGui::PopID();
		//	}
		//	else
		//	{
		//		ImPlot::SetupAxisLimits(ImAxis_Y1, m_hist_data_force_y_min - (m_hist_data_force_y_max - m_hist_data_force_y_min) / 10, m_hist_data_force_y_max + (m_hist_data_force_y_max - m_hist_data_force_y_min) / 10, ImGuiCond_Always);
		//	}
		//	{
		//		std::lock_guard<std::mutex> lock(m_vec_hist_data_force_lock);
		//		for (unsigned int i = 0; i < m_ForceSensor_num; i++)
		//		{
		//			if (m_slected_force_plot_index[i])
		//			{
		//				if (!m_vec_hist_data_force[i].Data.empty()) // �жϹ����������Ƿ���
		//				{
		//					ImPlot::PlotLine((u8"�������� " + std::to_string(i + 1)).c_str(), &m_vec_hist_data_force[i].Data[0].x, &m_vec_hist_data_force[i].Data[0].y,
		//						m_vec_hist_data_force[i].Data.size(), m_vec_hist_data_force[i].Offset, 2 * sizeof(float));
		//				}
		//			}
		//		}
		//	}
		//	ImPlot::EndPlot();
		//}
	}

	ImGui::End();
}