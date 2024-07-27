#include "UIHypersenMonitor.h"
#include "implot/implot.h"
#include "imgui/imgui.h"
#include "UIApplication.h"
#include "CPSAPI/CPSAPI.h"
#include "stb/stb_sprintf.h"
#include "UICfgParser.h"
#include "IconsFontAwesome5.h"
#include "GLFW/glfw3.h"
#include "UIUtils.h"


const char* HYPERSEN_DATA_LABEL[6] = {
	"Fx", "Fy", "Fz", "Mx", "My", "Mz"
};

UIHypersenMonitor::UIHypersenMonitor(UIGLWindow* main_win, const char* title):UIBaseWindow(main_win, title)
{
	m_vec_hist_data.resize(HYPERSEN_SENSOR_DOF);

	m_status_timer = new UITimer();
	m_status_timer->StartTimer(1.0);
}

UIHypersenMonitor::~UIHypersenMonitor()
{
	delete m_status_timer;
}

void UIHypersenMonitor::Draw()
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
	bool is_device_online = g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_hypersen_server_id);
	stbsp_sprintf(buf, "%s", is_device_online ? u8"在线" : u8"离线");
	ImGui::InputText(u8"服务状态", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

	if (is_device_online && !m_get_info_req_sent)
	{
		GetSensorInfo();
		m_get_info_req_sent = true;
	}

	if (ImGui::CollapsingHeader(u8"传感控制", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// 定时查询状态
		if (m_status_timer->CheckTimer())
		{
			GetSensorStatus();
		}
		//{
		//	std::lock_guard<std::mutex> lock(m_sensor_status_lock);
		//	if (m_sensor_status.status == 0)
		//	{
		//		if (ImGui::Button(u8"开始测量"))
		//		{
		//			StartMeasure();
		//		}
		//	}
		//	else if (m_sensor_status.status == 1)
		//	{
		//		if (ImGui::Button(u8"停止测量"))
		//		{
		//			StopMeasure();
		//		}
		//	}
		//	else
		//	{
		//		ImGui::Button(u8"传感器错误！");
		//	}
		//}
		if (ImGui::Button(u8"开始测量"))
		{
			StartMeasure();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"停止测量"))
		{
			StopMeasure();
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"传感器置零"))
		{
			ResetZero();
		}
	}

	if (ImGui::CollapsingHeader(u8"传感监控", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text(u8"传感器信息");
		{
			std::lock_guard<std::mutex> lock(m_sensor_info_lock);
			stbsp_sprintf(buf, "%d", m_sensor_info.dev_id);
			ImGui::InputText(u8"设备ID", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			stbsp_sprintf(buf, u8"%d年%d月%d日", m_sensor_info.version[0], m_sensor_info.version[1], m_sensor_info.version[2]);
			ImGui::InputText(u8"固件生成日期", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			stbsp_sprintf(buf, u8"V %d.%d.%d", m_sensor_info.version[3], m_sensor_info.version[4], m_sensor_info.version[5]);
			ImGui::InputText(u8"固件版本信息", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
		}
		ImGui::Text(u8"实时传感数据");
		{
			std::lock_guard<std::mutex> lock(m_cur_data_lock);
			stbsp_sprintf(buf, "%d", m_cur_data.code);
			ImGui::InputText(u8"Exception Code", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::InputFloat3(u8"Fx-Fy-Fz(N)", m_cur_data.data, "%.6f", ImGuiInputTextFlags_ReadOnly);
			ImGui::InputFloat3(u8"Mx-My-Mz(Nm)", m_cur_data.data + 3, "%.6f", ImGuiInputTextFlags_ReadOnly);
			float contact_force = (float)pow(m_cur_data.data[0] * m_cur_data.data[0] + m_cur_data.data[2] * m_cur_data.data[2], 0.5);
			ImGui::InputFloat(u8"contact force", &contact_force, 0.0f, 0.0f,"%.6f", ImGuiInputTextFlags_ReadOnly);
		}
		// plot hist
		{
			static float history = 10.0f;
			ImGui::SliderFloat(u8"Plot时长", &history, 1, 30, "%.1f s");

			static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
			// cur time
			float t = (float)glfwGetTime();

			if (ImPlot::BeginPlot(u8"传感器实时力数据", ImVec2(-1, 350)))
			{
				ImPlot::SetupAxes("t", "Force(N)", flags, flags);
				ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
				//ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
				{
					std::lock_guard<std::mutex> lock(m_vec_hist_data_lock);
					for (unsigned int i = 0; i < 3; i++)
					{
						if (!m_vec_hist_data[i].Data.empty())
						{
							ImPlot::PlotLine(HYPERSEN_DATA_LABEL[i], &m_vec_hist_data[i].Data[0].x, &m_vec_hist_data[i].Data[0].y,
								m_vec_hist_data[i].Data.size(), m_vec_hist_data[i].Offset, 2 * sizeof(float));
						}
					}
				}
				ImPlot::EndPlot();
			}

			if (ImPlot::BeginPlot(u8"传感器实时力矩数据", ImVec2(-1, 350)))
			{
				ImPlot::SetupAxes("t", "Torque(N*m)", flags, flags);
				ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
				//ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
				{
					std::lock_guard<std::mutex> lock(m_vec_hist_data_lock);
					for (unsigned int i = 3; i < HYPERSEN_SENSOR_DOF; i++)
					{
						if (!m_vec_hist_data[i].Data.empty())
						{
							ImPlot::PlotLine(HYPERSEN_DATA_LABEL[i], &m_vec_hist_data[i].Data[0].x, &m_vec_hist_data[i].Data[0].y,
								m_vec_hist_data[i].Data.size(), m_vec_hist_data[i].Offset, 2 * sizeof(float));
						}
					}
				}
				ImPlot::EndPlot();
			}
		}
	}
	if (ImGui::CollapsingHeader(u8"数据存储", ImGuiTreeNodeFlags_DefaultOpen))
	{
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
					UI_INFO(u8"开始采集单点 Force Sensor数据到文件%s", filename);
					if (fp)
					{
						std::lock_guard<std::mutex> lock(m_cur_data_lock);
						for (unsigned int j = 0; j < HYPERSEN_SENSOR_DOF; j++)
						{
							fprintf(fp, "%.2f", m_cur_data.data[j]);
							if (j != HYPERSEN_SENSOR_DOF - 1)
							{
								fprintf(fp, ",");
							}
						}
						fprintf(fp, "\n");
						m_start_singlepoint_sampling = false;
						UI_INFO(u8"结束单点采集Force Sensor数据!");
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

void UIHypersenMonitor::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case MSG_HYPERSEN_SENSOR_INFO:
		OnSensorInfo(from_id, data, msg_len);
		break;
	case MSG_HYPERSEN_SENSOR_DATA:
		OnSensorData(from_id, data, msg_len);
		break;
	case MSG_HYPERSEN_STATUS:
		OnSensorStatus(from_id, data, msg_len);
		break;
	}
}

void UIHypersenMonitor::OnSensorStatus(uint32_t from_id, const char* data, uint32_t msg_len)
{
	std::lock_guard<std::mutex> lock(m_sensor_status_lock);
	m_sensor_status = *(ST_HypersenSensorStatus*)data;
}

void UIHypersenMonitor::OnSensorInfo(uint32_t from_id, const char* data, uint32_t msg_len)
{
	std::lock_guard<std::mutex> lock(m_sensor_info_lock);
	m_sensor_info = *(ST_HypersenSensorInfo*)data;
}

void UIHypersenMonitor::OnSensorData(uint32_t from_id, const char* data, uint32_t msg_len)
{
	{
		std::lock_guard<std::mutex> lock(m_cur_data_lock);
		m_cur_data = *(ST_HypersenSensorData*)data;
	}
	{
		float t = (float)glfwGetTime();
		std::lock_guard<std::mutex> lock(m_vec_hist_data_lock);
		for (unsigned int i = 0; i < HYPERSEN_SENSOR_DOF; i++)
		{
			m_vec_hist_data[i].AddPoint(t, m_cur_data.data[i]);
		}
	}
}

void UIHypersenMonitor::StartMeasure()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_hypersen_server_id))
	{
		UI_ERROR(u8"设备[%d]不在线！", g_cfg->m_hypersen_server_id);
		return;
	}
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_hypersen_server_id, MSG_HYPERSEN_START_READ, NULL, 0);
}

void UIHypersenMonitor::StopMeasure()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_hypersen_server_id))
	{
		UI_ERROR(u8"设备[%d]不在线！", g_cfg->m_hypersen_server_id);
		return;
	}
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_hypersen_server_id, MSG_HYPERSEN_STOP_READ, NULL, 0);
}

void UIHypersenMonitor::ResetZero()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_hypersen_server_id))
	{
		UI_ERROR(u8"设备[%d]不在线！", g_cfg->m_hypersen_server_id);
		return;
	}
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_hypersen_server_id, MSG_HYPERSEN_RESET_ZERO, NULL, 0);
}

void UIHypersenMonitor::GetSensorInfo()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_hypersen_server_id))
	{
		UI_ERROR(u8"设备[%d]不在线！", g_cfg->m_hypersen_server_id);
		return;
	}
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_hypersen_server_id, MSG_HYPERSEN_GET_SENSOR_INFO, NULL, 0);
}

void UIHypersenMonitor::GetSensorStatus()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_hypersen_server_id))
	{
		return;
	}
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_hypersen_server_id, MSG_HYPERSEN_READ_STATUS, NULL, 0);
}
