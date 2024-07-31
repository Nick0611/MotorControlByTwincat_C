#include "UIForceSensor_HUATRAN.h"
#include "GLFW/glfw3.h" // 必須在cpp文件中include
#include <cmath>
#include "nlohmann/json.hpp"
#include "TimeCounter.h"

void UIForceSensor_HUATRAN::Thread_ReadForceData()
{
	long      nErr, nPort;	//定义端口变量
	AmsAddr   Addr;		//定义AMS地址变量
	PAmsAddr  pAddr = &Addr;//定义端口地址变量
	unsigned long pcbReturn;
	nPort = AdsPortOpenEx();		//打开ADS通讯端口
	if (ads_local_or_CX_flag)
	{
		pAddr->netId.b[0] = m_pAddr->netId.b[0];  //手动填写目标设备的AMSNETID
		pAddr->netId.b[1] = m_pAddr->netId.b[1]; //在例子中设备通讯AMSNETID为5.23.20.78.1.1
		pAddr->netId.b[2] = m_pAddr->netId.b[2];
		pAddr->netId.b[3] = m_pAddr->netId.b[3];
		pAddr->netId.b[4] = m_pAddr->netId.b[4];
		pAddr->netId.b[5] = m_pAddr->netId.b[5];
	}
	else
	{
		long nErr = AdsGetLocalAddressEx(nPort, pAddr); //自动获取本地地址
		if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
	}
	pAddr->port = 851;			//TC3的通讯端口为851
	unsigned long lHdlVar;   	//创建句柄
	char szVar_pos[] = { "main.force" };
	while (!thread_read_force_stop_flag)
	{
		auto startTime = std::chrono::high_resolution_clock::now();
		float* myArray = new float[m_ForceSensor_num];
		nErr = AdsSyncReadWriteReqEx2(nPort, pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, sizeof(szVar_pos), szVar_pos, &pcbReturn);
		if (nErr) UI_ERROR("1Error: AdsSyncReadWriteReq: %ld", nErr);
		{
			// 注意：不能直接sizeof(myArray)，大小不对！！！
			nErr = AdsSyncReadReqEx2(nPort, pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(float) * m_ForceSensor_num, &myArray[0], &pcbReturn); //这个读的是固定大小的数组哇
			if (nErr) { UI_ERROR("Error: AdsSyncReadReq: %ld", nErr); }
			else
			{
				std::lock_guard<std::mutex> lock(m_thread_read_force_lock);
				for (unsigned int i = 0; i < m_ForceSensor_num; i++)//用for循环语句来实现读取数组中的元素
				{
					float t = (float)glfwGetTime();
					m_vec_hist_data_force[i].AddPoint(t, myArray[i]);
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

bool UIForceSensor_HUATRAN::LoadCfg()
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
			// 解析 JSON 数据
			nlohmann::json config;
			ifs >> config;

			m_ReadFrequency_hz = config["ReadFrequency_hz"];
			// 获取 "TargetSource" 的值
			// 根据 "TargetSource" 的值执行相应的操作
			std::string targetSource = config["TargetSource"];
			if (targetSource == "Local") {
				// 无其他操作
				ads_local_or_CX_flag = false;
			}
			else if (config.find(targetSource) != config.end()) {
				// 读取相应子项的内容
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

UIForceSensor_HUATRAN::UIForceSensor_HUATRAN(UIGLWindow* main_win, const char* title) :UIBaseWindow(main_win, title)
{
	LoadCfg();
	m_slected_force_plot_index.resize(m_ForceSensor_num);
	m_vec_hist_data_force.resize(m_ForceSensor_num);
	m_vec_hist_data_force_y_min.resize(m_ForceSensor_num, -1);
	m_vec_hist_data_force_y_max.resize(m_ForceSensor_num, 1);
	m_hist_data_force_y_min = -1;
	m_hist_data_force_y_min = 1;

	m_record_flag_continuous = false;
	m_record_flag_jog = false;
	m_record_jog_num = 0;
	m_selected_force_plot_all = false;
}

UIForceSensor_HUATRAN::~UIForceSensor_HUATRAN()
{
	{
		std::lock_guard<std::mutex> lock(m_thread_read_force_lock);
		thread_read_force_stop_flag = true;
	}

	if (thread_read_force.joinable()) thread_read_force.join();
	AdsPortCloseEx(m_nPort);
}

void UIForceSensor_HUATRAN::Draw()
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

	// 0. 是否启动该模块，包括1个ADS读线程和一个全局ADS线程
	char buf_module_enable[256] = { 0 };
	stbsp_sprintf(buf_module_enable, u8"%s 启动该模块", ICON_FA_MERCURY);
	bool tmp = ImGui::Checkbox(buf_module_enable, &Enable_this_module);
	if (tmp && Enable_this_module)
	{
		{
			std::lock_guard<std::mutex> lock(m_thread_read_force_lock);
			thread_read_force_stop_flag = false;
		}
		m_nPort = AdsPortOpenEx();		//打开ADS通讯端口
		if (!ads_local_or_CX_flag)
		{
			long nErr = AdsGetLocalAddressEx(m_nPort, m_pAddr); //自动获取本地地址
			if (nErr) UI_ERROR("Error: AdsGetLocalAddress: %ld", nErr);
		}
		m_pAddr->port = 851;			//TC3的通讯端口为851
		if (!thread_read_force.joinable())
		{
			thread_read_force = std::thread(&UIForceSensor_HUATRAN::Thread_ReadForceData, this);
		}
	}
	if (!Enable_this_module)
	{
		{
			std::lock_guard<std::mutex> lock(m_thread_read_force_lock);
			thread_read_force_stop_flag = true;
		}
		if (thread_read_force.joinable()) thread_read_force.join();
		AdsPortCloseEx(m_nPort);
	}

	ImGui::End();
}