#include "UIOptiTrack.h"
#include "UIApplication.h"
#include <CPSAPI/CPSAPI.h>
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include "IconsFontAwesome5.h"
#include "UICfgParser.h"
#include "stb/stb_sprintf.h"
#include "UserTool.h"

UIOptiTrack::UIOptiTrack(UIGLWindow* main_win, const char* title):UIBaseWindow(main_win, title)
{
	m_selected_index.resize(m_disp_marker_count, -1);
	//m_marker_list.marker_num = 15;
	//for (int i = 0; i < m_marker_list.marker_num; i++)
	//{
	//	m_marker_list.markers[i].ID = i+1;
	//}
}

UIOptiTrack::~UIOptiTrack()
{
}

void UIOptiTrack::Draw()
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
	bool is_device_online = g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_opt_server_id);
	stbsp_sprintf(buf, "%s", is_device_online ? u8"在线" : u8"离线");
	ImGui::InputText(u8"服务状态", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
	
	if (ImGui::CollapsingHeader(u8"Marker监控", ImGuiTreeNodeFlags_DefaultOpen))
	{
		stbsp_snprintf(buf, sizeof(buf), u8"%s 初始化服务", ICON_FA_PLAY);
		if (ImGui::Button(buf))
		{
			InitOptService();
		}
		ImGui::SameLine();
		stbsp_snprintf(buf, sizeof(buf), u8"%s 停止服务", ICON_FA_STOP);
		if (ImGui::Button(buf))
		{
			StopOptService();
		}

		stbsp_snprintf(buf, sizeof(buf), u8"%s Marker数量", ICON_FA_LIST_OL);
		ImGui::Text(buf);
		if (ImGui::SliderInt("N[1-15]", &m_disp_marker_count, 1, MAX_MARKER_NUM))
		{
			m_selected_index.resize(m_disp_marker_count, -1);
		}

		for (int i = 0; i < m_disp_marker_count; i++)
		{
			stbsp_snprintf(buf, sizeof(buf), "Marker_%d", i);
			char preview_value[64] = { 0 };
			if (m_selected_index[i] >= 0)
			{
				std::lock_guard<std::mutex> lock(m_marker_list_lock);
				stbsp_snprintf(preview_value, sizeof(preview_value), "%d", m_marker_list.markers[m_selected_index[i]].ID);
			}
			if (ImGui::BeginCombo(buf, preview_value, 0))
			{
				{
					std::lock_guard<std::mutex> lock(m_marker_list_lock);
					for (int n = 0; n < m_marker_list.marker_num; n++)
					{
						const bool is_selected = (m_selected_index[i] == n);
						if (ImGui::Selectable(std::to_string(m_marker_list.markers[n].ID).c_str(), is_selected))
							m_selected_index[i] = n;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			// display XYZ
			ImGui::PushID(i);
			if (m_selected_index[i] >= 0)
			{
				std::lock_guard<std::mutex> lock(m_marker_list_lock);
				ImGui::InputFloat3("XYZ", m_marker_list.markers[m_selected_index[i]].XYZ, "%.3f", ImGuiInputTextFlags_ReadOnly);
			}
			else
			{
				float XYZ[3] = { 0 };
				ImGui::InputFloat3("XYZ", XYZ, "%.3f", ImGuiInputTextFlags_ReadOnly);
			}
			ImGui::PopID();
		}
	}
	if (ImGui::CollapsingHeader(u8"数据存储", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static bool is_manual = true;

		if (ImGui::RadioButton(u8"单次存储Marker点数据", is_manual))
		{
			is_manual = true;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton(u8"连续存储Marker点数据", !is_manual))
		{
			is_manual = false;
		}

		stbsp_snprintf(buf, sizeof(buf), u8"%s 重置Marker信息", ICON_FA_UNDO);
		if (ImGui::Button(buf))
		{
			ResetMarkerInfo();
		}
		ImGui::SameLine();
		if (is_manual)
		{
			stbsp_snprintf(buf, sizeof(buf), u8"%s 保存选择Marker点", ICON_FA_SAVE);
			if (ImGui::Button(buf))
			{
				SaveSelectedMarkers();
			}
			ImGui::SameLine();
			stbsp_snprintf(buf, sizeof(buf), u8"%s 保存所有Marker点", ICON_FA_SAVE);
			if (ImGui::Button(buf))
			{
				SaveAllMarkers();
			}
		}
		else
		{
			if (!m_start_serialize)
			{
				stbsp_sprintf(buf, u8"%s 开始采集", ICON_FA_PLAY_CIRCLE);
				if (ImGui::Button(buf))
				{
					if (is_device_online)
						m_start_serialize = true;
					else
						UI_WARN(u8"设备[%d]不在线！", g_cfg->m_opt_server_id);
				}
			}
			else
			{
				stbsp_sprintf(buf, u8"%s 停止采集", ICON_FA_STOP_CIRCLE);
				if (ImGui::Button(buf))
				{
					m_start_serialize = false;
					SaveVecData();
				}
			}
		}
	}
	ImGui::End();
}

void UIOptiTrack::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case MSG_OPTITRACK_MARKER_INFO:
		OnMarkerListMsg(from_id, msg_type, data, msg_len);
		break;
	case MSG_CMD_INIT_RSP:
		OnInitRsp(from_id, msg_type, data, msg_len);
		break;
	case MSG_CMD_STOP_RSP:
		OnStopRsp(from_id, msg_type, data, msg_len);
		break;
	}
}

void UIOptiTrack::OnInitRsp(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	ST_CMDInitRsp* rsp = (ST_CMDInitRsp*)data;
	if (rsp->rsp.error_code == 0)
	{
		UI_INFO(u8"[%d]-OptiTrack 服务初始化成功！", rsp->req_no);
	}
	else
	{
		UI_ERROR(u8"[%d]-OptiTrack 服务初始化失败！原因：[%d]-%s", rsp->req_no, rsp->rsp.error_code, rsp->rsp.error_msg);
	}
}

void UIOptiTrack::OnStopRsp(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	ST_CMDStopRsp* rsp = (ST_CMDStopRsp*)data;
	if (rsp->rsp.error_code == 0)
	{
		UI_INFO(u8"[%d]-OptiTrack 服务停止成功！", rsp->req_no);
	}
	else
	{
		UI_ERROR(u8"[%d]-OptiTrack 服务停止失败！原因：[%d]-%s", rsp->req_no, rsp->rsp.error_code, rsp->rsp.error_msg);
	}
}

void UIOptiTrack::OnMarkerListMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	{
		std::lock_guard<std::mutex> lock(m_marker_list_lock);
		memcpy(&m_marker_list, data, sizeof(ST_OptMarker_List));
	}
	if (m_start_serialize)
	{
		std::lock_guard<std::mutex> lock(m_vec_data_lock);
		for (int i = 0; i < m_marker_list.marker_num; i++)
		{
			if (m_marker_list.markers[i].ID > 0)
			{
				m_vec_data[m_marker_list.markers[i].ID].push_back(m_marker_list.markers[i]);
			}
		}
	}
}

void UIOptiTrack::SaveSelectedMarkers()
{
	if (!std::any_of(m_selected_index.begin(), m_selected_index.end(), 
		[](auto& idx) {	return idx >= 0; }))
	{
		UI_WARN(u8"没有选择Marker点，保存失败！");
		return;
	}
	// get current time
	char buf[256] = { 0 };
	char dt[64] = { 0 };
	GetTimeStr(dt, sizeof(dt));
	stbsp_snprintf(buf, sizeof(buf), "marker_info_selected_%s.txt", dt);

	std::ofstream os(buf);
	if (!os.is_open())
	{
		UI_WARN(u8"打开文件%失败！", buf);
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_marker_list_lock);
		for (auto i : m_selected_index)
		{
			if (i >= 0)
			{
				os << m_marker_list.markers[i].ID << ", "
					<< m_marker_list.markers[i].XYZ[0] << ", "
					<< m_marker_list.markers[i].XYZ[1] << ", "
					<< m_marker_list.markers[i].XYZ[2] << "\n";
			}
		}
	}
	UI_INFO(u8"保存选择点到文件%s成功！", buf);
	os.close();
}

void UIOptiTrack::SaveAllMarkers()
{
	if (m_marker_list.marker_num <= 0)
	{
		UI_WARN(u8"没有Marker点数据，保存失败！");
		return;
	}
	// get current time
	char buf[256] = { 0 };
	char dt[64] = { 0 };
	GetTimeStr(dt, sizeof(dt));
	stbsp_snprintf(buf, sizeof(buf), "marker_info_all_%s.txt", dt);

	std::ofstream os(buf);
	if (!os.is_open())
	{
		UI_WARN(u8"打开文件%失败！", buf);
		return;
	}
	{
		std::lock_guard<std::mutex> lock(m_marker_list_lock);
		for (int i = 0; i < m_marker_list.marker_num; i++)
		{
			os << m_marker_list.markers[i].ID << ", "
				<< m_marker_list.markers[i].XYZ[0] << ", "
				<< m_marker_list.markers[i].XYZ[1] << ", "
				<< m_marker_list.markers[i].XYZ[2] << "\n";
		}
	}
	UI_INFO(u8"保存所有点到文件%s成功！", buf);
	os.close();
}

void UIOptiTrack::SaveVecData()
{// UI线程调用
	std::map<int, std::vector<ST_OptMarker>> vec_data;
	{
		std::lock_guard<std::mutex> lock(m_vec_data_lock);
		m_vec_data.swap(vec_data);
	}

	for (int i = 0; i < m_disp_marker_count; i++)
	{
		int id = m_marker_list.markers[m_selected_index[i]].ID;
		auto iter = vec_data.find(id);
		if (iter != vec_data.end())
		{
			SaveVecMarker(id, iter->second);
		}
	}
	ResetCache();
}


void UIOptiTrack::SaveVecMarker(int id, const std::vector<ST_OptMarker>& vec)
{
	if (vec.empty())
	{
		UI_WARN(u8"没有Marker点[%d]数据，保存失败！", id);
		return;
	}
	// get current time
	char buf[256] = { 0 };
	char dt[64] = { 0 };
	GetTimeStr(dt, sizeof(dt));
	stbsp_snprintf(buf, sizeof(buf), "marker_vec_info_%d_%s.txt", id, dt);

	std::ofstream os(buf);
	if (!os.is_open())
	{
		UI_WARN(u8"打开文件%失败！", buf);
		return;
	}
	{
		for (size_t i = 0; i < vec.size(); i++)
		{
			os << vec[i].ID << ", "
				<< vec[i].XYZ[0] << ", "
				<< vec[i].XYZ[1] << ", "
				<< vec[i].XYZ[2] << "\n";
		}
	}
	UI_INFO(u8"保存Marker点[%d]%d个数据到文件%s成功！", id, vec.size(), buf);
	os.close();
}

void UIOptiTrack::ResetCache()
{
	std::lock_guard<std::mutex> lock(m_vec_data_lock);
	m_vec_data.clear();
}

void UIOptiTrack::ResetMarkerInfo()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_opt_server_id))
	{
		UI_ERROR(u8"设备[%d]不在线！", g_cfg->m_opt_server_id);
		return;
	}
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_opt_server_id, MSG_RESET_MARKER_ID, NULL, 0);
}

void UIOptiTrack::InitOptService()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_opt_server_id))
	{
		UI_ERROR(u8"设备[%d]不在线！", g_cfg->m_opt_server_id);
		return;
	}
	ST_CMDInit req = { m_req_id++ };
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_opt_server_id, MSG_CMD_INIT, (const char*)&req, sizeof(ST_CMDInit));
}

void UIOptiTrack::StopOptService()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_opt_server_id))
	{
		UI_ERROR(u8"设备[%d]不在线！", g_cfg->m_opt_server_id);
		return;
	}
	ST_CMDStop req = { m_req_id++ };
	g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_opt_server_id, MSG_CMD_STOP, (const char*)&req, sizeof(ST_CMDStop));
}

