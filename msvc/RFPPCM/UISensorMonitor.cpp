#include "UISensorMonitor.h"
#include "implot/implot.h"
#include "imgui/imgui.h"
#include "stb/stb_sprintf.h"
#include "UIApplication.h"
#include "UICfgParser.h"
#include "CPSAPI/CPSAPI.h"
#include "UserTool.h"
#include "IconsFontAwesome5.h"
#include "GLFW/glfw3.h"


UISensorMonitor::UISensorMonitor(UIGLWindow* main_win, const char* title): UIBaseWindow(main_win, title)
{
	m_serialize_thread = std::move(std::thread(&UISensorMonitor::SerializeFunc, this));

	m_vec_hist_data.resize(m_max_line_plot_num);
}

UISensorMonitor::~UISensorMonitor()
{
	m_exiting = true;
	if (m_serialize_thread.joinable())
	{
		m_serialize_thread.join();
	}
}

void UISensorMonitor::Draw()
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
	char buf[64] = { 0 };
	bool is_device_online = g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_sensor_server_id);
	stbsp_sprintf(buf, "%s", is_device_online ? u8"在线" : u8"离线");
	ImGui::InputText(u8"服务状态", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

	if (ImGui::CollapsingHeader(u8"传感监控", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text(u8"实时传感数据");

		{
			std::lock_guard<std::mutex> lock(m_cur_data_lock);
			stbsp_sprintf(buf, "%d", m_cur_data.sensor_num);
			ImGui::InputText(u8"数据数量", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::InputFloat4(u8"实时数据", m_cur_data.data, "%.3f", ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s 置零", ICON_FA_UNDO);
			if (ImGui::Button(buf))
				reset_initial_value = true;				
		}
		// plot hist
		{
			static float history = 10.0f;
			ImGui::SliderFloat(u8"Plot时长", &history, 1, 30, "%.1f s");

			static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
			// cur time
			float t = (float)glfwGetTime();
			
			if (ImPlot::BeginPlot("##sensor_plot", ImVec2(-1, 350)))
			{
				ImPlot::SetupAxes(NULL, NULL, flags, flags);
				ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
				//ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
				{
					std::lock_guard<std::mutex> lock(m_vec_hist_data_lock);
					std::lock_guard<std::mutex> lock2(m_cur_data_lock);
					for (unsigned int i = 0; i < m_max_line_plot_num && i < m_cur_data.sensor_num; i++)
					{
						stbsp_sprintf(buf, "L%d", i);
						ImPlot::PlotLine(buf, &m_vec_hist_data[i].Data[0].x, &m_vec_hist_data[i].Data[0].y, 
							m_vec_hist_data[i].Data.size(), m_vec_hist_data[i].Offset, 2 * sizeof(float));
					}
				}
				ImPlot::EndPlot();
			}
		}
	}
	if (ImGui::CollapsingHeader(u8"数据存储", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//连续采集
		if (!m_start_serialize)
		{
			stbsp_sprintf(buf, u8"%s 开始采集", ICON_FA_PLAY_CIRCLE);
			if (ImGui::Button(buf))
			{
				if (is_device_online)
					m_start_serialize = true;
				else
					UI_WARN(u8"设备[%d]不在线！", g_cfg->m_sensor_server_id);
			}
		}
		else
		{
			stbsp_sprintf(buf, u8"%s 停止采集", ICON_FA_STOP_CIRCLE);
			if (ImGui::Button(buf))
			{
				m_start_serialize = false;
			}
		}
		//单点采集
		ImGui::SameLine();
		if (!m_start_singlepoint_sampling)
		{
			stbsp_sprintf(buf, u8"%s 单点采集", ICON_FA_PLAY_CIRCLE);
			if (ImGui::Button(buf))
			{
				if (is_device_online)
				{
					m_start_singlepoint_sampling = true;
					FILE* fp = NULL;
					fp = fopen(filename, "a+");
					UI_INFO(u8"开始采集单点Sensor数据到文件%s", filename);
					if (fp)
					{
						std::lock_guard<std::mutex> lock(m_cur_data_lock);
						for (unsigned int j = 0; j < m_cur_data.sensor_num; j++)
						{
							fprintf(fp, "%.2f", m_cur_data.data[j]);
							if (j != m_cur_data.sensor_num - 1)
							{
								fprintf(fp, ",");
							}
						}
						fprintf(fp, "\n");
						m_start_singlepoint_sampling = false;
						UI_INFO(u8"结束单点采集Sensor数据!");
					}
					if (fp)
					{
						fclose(fp);
					}
				}					
				else
					UI_WARN(u8"设备[%d]不在线！", g_cfg->m_sensor_server_id);
			}
		}
		else
		{
			stbsp_sprintf(buf, u8"%s 单点采集", ICON_FA_PLAY_CIRCLE);
			if (ImGui::Button(buf))
			{
				m_start_singlepoint_sampling = false;
			}
		}
		ImGui::SameLine();		
		ImGui::InputTextWithHint(u8"单点采集文件名", "enter filename here", filename, IM_ARRAYSIZE(filename));
	}
	ImGui::End();
}

void UISensorMonitor::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case MSG_SENSOR_DATA:
		OnSensorData(from_id, data, msg_len);
		break;
	}
}

void UISensorMonitor::GetCurrentSensorData(ST_SensorData& data)
{
	std::lock_guard<std::mutex> lock(m_cur_data_lock);
	data = m_cur_data;
}

void UISensorMonitor::OnSensorData(uint32_t from_id, const char* data, uint32_t msg_len)
{
	{
		std::lock_guard<std::mutex> lock(m_cur_data_lock);
		m_cur_data = *(ST_SensorData*)data;
		char receive_t[64] = { 0 };
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		snprintf(receive_t, 64, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		strcpy(m_cur_data.t2, receive_t);
		if(reset_initial_value)
		{
			memcpy(this->initial_value, m_cur_data.data, sizeof(m_cur_data.data));
			reset_initial_value = false;
		}		
		for (int i = 0; i < m_cur_data.sensor_num; i++)
		{
			m_cur_data.data[i] = m_cur_data.data[i] - initial_value[i];
		}
	}
	if (m_start_serialize)
	{
		std::lock_guard<std::mutex> lock(m_vec_data_lock);
		m_vec_data.push_back(m_cur_data);
	}
	{ // 绘图专用
		float t = (float)glfwGetTime();

		std::lock_guard<std::mutex> lock(m_vec_hist_data_lock);
		for (unsigned int i = 0; i < m_max_line_plot_num && i < m_cur_data.sensor_num; i++)
		{
			m_vec_hist_data[i].AddPoint(t, m_cur_data.data[i]);
		}
	}
}

void UISensorMonitor::ResetCache()
{
	std::lock_guard<std::mutex> lock(m_vec_data_lock);
	m_vec_data.clear();
}

void UISensorMonitor::RetrieveAllData(std::vector<ST_SensorData>& vec_data)
{
	std::lock_guard<std::mutex> lock(m_vec_data_lock);
	m_vec_data.swap(vec_data);
}

void UISensorMonitor::SerializeFunc(void* arg)
{
	UISensorMonitor* ui = (UISensorMonitor*)arg;
	FILE* fp = NULL;

	while (!ui->m_exiting)
	{
		if (!ui->m_start_serialize)
		{
			if (fp)
			{
				fclose(fp);
				fp = NULL;
				UI_INFO(u8"结束采集Sensor数据!");
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		if (!fp)
		{
			//连续采集
			if (ui->m_start_serialize)
			{
				ui->ResetCache();

				char filename[256] = { 0 };
				char dt[64] = { 0 };
				GetTimeStr(dt, sizeof(dt));
				snprintf(filename, sizeof(filename), "sensor_data_%s.txt", dt);
				fp = fopen(filename, "wb");
				UI_INFO(u8"开始采集Sensor数据到文件%s", filename);
			}
			//单点采集
			//if (ui->m_start_singlepoint_sampling)
			//{
			//	//char filename[256] = { 0 };
			//	//std::string str = "sensor_data_single_point";
			//	//strcpy(filename, str.c_str());
			//	UI_INFO(u8"开始采集单点数据");
			//	fp = fopen(ui->filename, "a+");
			//	UI_INFO(u8"开始采集单点Sensor数据到文件%s", ui->filename);
			//	if (fp)
			//	{
			//		std::lock_guard<std::mutex> lock(ui->m_cur_data_lock);
			//		for (unsigned int j = 0; j < ui->m_cur_data.sensor_num; j++)
			//		{
			//			fprintf(fp, "%.2f", ui->m_cur_data.data[j]);
			//			if (j != ui->m_cur_data.sensor_num - 1)
			//			{
			//				fprintf(fp, ",");
			//			}
			//		}
			//		fprintf(fp, "\n");
			//		ui->m_start_singlepoint_sampling = false;
			//		UI_INFO(u8"结束单点采集Sensor数据!");
			//	}
			//}
		}
		// 等待一段时间
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::vector<ST_SensorData> vec_data;
		ui->RetrieveAllData(vec_data);
		for (size_t i = 0; i < vec_data.size(); i++)
		{
			if (fp)
			{
				for (unsigned int j = 0; j < vec_data[i].sensor_num; j++)
				{
					fprintf(fp, "%.2f", vec_data[i].data[j]);
					if (j != vec_data[i].sensor_num - 1)
					{
						fprintf(fp, ",");
					}
				}
				fprintf(fp, "\n");
			}
		}
		if(!vec_data.empty()) UI_INFO(u8"写入%d条数据完成!", vec_data.size());
	}
	if (fp)
	{
		fclose(fp);
	}
}
