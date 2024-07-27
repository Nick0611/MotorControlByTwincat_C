#include "UIFinRay.h"
#include "CPSAPI/CPSAPI.h"
#include "UIApplication.h"
#include "stb/stb_sprintf.h"
#include "implot/implot.h"
#include "imgui/imgui.h"
#include "IconsFontAwesome5.h"
#include "CPSSensorServerDef.h"
#include "UIWindowManager.h"
#include "UIMainWindow.h"
#include "UICfgParser.h"
#include "GLFW/glfw3.h"

const char* SENSING_FORCE_LAABLE[2] = {
	"Sensing", "Desired"
};

UIFinRay::UIFinRay(UIGLWindow* main_win, const char* title): UIBaseWindow(main_win, title)
{
	m_epos2 = std::make_shared<Device::EPOS2>(m_epos_node_id);
	calibration_parameter = this->readMatrixFile("calibration.txt");
	m_vec_hist_data.resize(2);
}

void UIFinRay::UpdatePlotData(const std::string& key1, const std::vector<double>& x, const std::vector<double>& y, const std::string& key2, const std::vector<double>& rbx, const std::vector<double>& rby)
{
	m_map_plot_shape_data[key1].x = x;
	m_map_plot_shape_data[key1].y = y;

	m_map_plot_rib_data[key2].rib1x.assign(rbx.begin(), rbx.begin() + 2);
	m_map_plot_rib_data[key2].rib1y.assign(rby.begin(), rby.begin() + 2);

	m_map_plot_rib_data[key2].rib2x.assign(rbx.begin() + 2, rbx.begin() + 4);
	m_map_plot_rib_data[key2].rib2y.assign(rby.begin() + 2, rby.begin() + 4);

	m_map_plot_rib_data[key2].rib3x.assign(rbx.begin() + 4, rbx.begin() + 6);
	m_map_plot_rib_data[key2].rib3y.assign(rby.begin() + 4, rby.begin() + 6);

	m_map_plot_rib_data[key2].rib4x.assign(rbx.begin() + 6, rbx.begin() + 8);
	m_map_plot_rib_data[key2].rib4y.assign(rby.begin() + 6, rby.begin() + 8);

	m_map_plot_rib_data[key2].rib5x.assign(rbx.begin() + 8, rbx.begin() + 10);
	m_map_plot_rib_data[key2].rib5y.assign(rby.begin() + 8, rby.begin() + 10);
}

void UIFinRay::Draw()
{
	if (!m_show)
	{
		return;
	}
	if (!ImGui::Begin(m_win_title, &m_show/*, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse*/))
	{
		ImGui::End();
		return;
	}

	char buf[1024] = { 0 };

	stbsp_sprintf(buf, u8"%s %s", m_is_running ? ICON_FA_STOP : ICON_FA_PLAY, m_is_running ? u8"停止力感知" : u8"开始力感知");
	if (ImGui::Button(buf))
	{
		m_is_running = !m_is_running;
		if (!m_is_running)
		{
			m_epos2->Halt();
		}
	}
	if (m_is_running)
	{
		ExecuteForceFeedBackPolicy();
	}
	static float KKp = 1.00f, KKi = 0.00f, KKd = 0.00f;
	ImGui::SliderFloat("Kp(0~30)", &KKp, 0.0f, 30.0f, "Kp = %.2f");
	ImGui::SameLine;
	ImGui::InputFloat("Input Kp", &KKp, 0.01f, 0.1f, "%.3f");
	ImGui::SliderFloat("Ki(0~30)", &KKi, 0.0f, 30.0f, "Ki = %.2f");
	ImGui::SameLine;
	ImGui::InputFloat("Input Ki", &KKi, 0.01f, 0.1f, "%.3f");
	ImGui::SliderFloat("Kd(0~30)", &KKd, 0.0f, 30.0f, "Kd = %.2f");
	ImGui::SameLine;
	ImGui::InputFloat("Input Kd", &KKd, 0.01f, 0.1f, "%.3f");
	Kp = KKp;
	Ki = KKi;
	Kd = KKd;

	if (ImGui::Button(u8"发送计算请求"))
	{
		SendCalcReq();
	}
	ImGui::SameLine();
	static bool check = false;
	ImGui::Checkbox(u8"实时计算", &check);
	if (check)
	{
		SendCalcReq();
	}
	static bool check_t = false;
	ImGui::Checkbox(u8"时间记录", &check_t);
	ImGui::SameLine();
	ImGui::InputTextWithHint(u8"时间流记录", "enter filename here", filename, IM_ARRAYSIZE(filename));
	
	std::lock_guard<std::mutex> lock(m_calc_result_lock);

	if(check_t)
	{
		is_start_record_time = true;
	}
	else
	{
		is_start_record_time = false;
	}


	stbsp_sprintf(buf, u8"%s 力感知", ICON_FA_DESKTOP);
	if (ImGui::CollapsingHeader(buf, ImGuiTreeNodeFlags_DefaultOpen))
	{
		stbsp_sprintf(buf, u8"%s FinRay外力感知", ICON_FA_SATELLITE);
		ImGui::Text(buf);

		float force = (float)m_calc_result.contact_force;
		ImGui::InputFloat(u8"接触力(N)", &force, 0.0f, 0.0f, "%.6f", ImGuiInputTextFlags_ReadOnly);

		float force_pos[3], force_dir[3],force_dirx[2],force_diry[2],contact_posx[1], contact_posy[1],fsign=1;
		if (force > 0)
			fsign = 1;
		else
			fsign = 1;
		for (int i = 0; i < 3; i++)
		{
			force_pos[i] = (float)m_calc_result.contact_position[i];
			force_dir[i] = (float)m_calc_result.contact_direction[i];
		}
		contact_posx[0] = force_pos[0];
		contact_posy[0] = force_pos[1];

		//ImGui::InputFloat3(u8"接触力位置(mm)", force_pos, "%.6f", ImGuiInputTextFlags_ReadOnly);
		//ImGui::InputFloat3(u8"接触力方向", force_dir, "%.6f", ImGuiInputTextFlags_ReadOnly);
		static float f_desired = 1;// desired force
		ImGui::SliderFloat("Desired force(N)", &f_desired, 0.0f, 10.0f, "fd = %.1f");
		fd = f_desired;

		static bool check2 = false;
		ImGui::Checkbox(u8"力感知数据记录", &check2);
		ImGui::SameLine();
		ImGui::InputTextWithHint(u8"力记录", "enter filename here", force_data_file, IM_ARRAYSIZE(force_data_file));
		if (check2)
		{
			float tt = (float)glfwGetTime();
			fp_force = fopen(force_data_file, "a+");
			if (fp_force)
			{
				//std::lock_guard<std::mutex> lock(m_cur_data_lock);
				fprintf(fp_force, "t=%.6f:%.3f,%.3f", tt, force, fd);
				fprintf(fp_force, "\n");
			}
			if (fp_force)
			{
				fclose(fp_force);
			}
		}

		static float history = 10.0f;
		ImGui::SliderFloat(u8"Plot时长", &history, 1, 30, "%.1f s");
		static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
		float t = (float)glfwGetTime();
		if (ImPlot::BeginPlot(u8"感知力与目标力数据", ImVec2(-1, 350)))
		{
			ImPlot::SetupAxes("t(s)", "Force(N)", flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
			//ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
			{
				std::lock_guard<std::mutex> lock(m_vec_hist_data_lock);
				for (unsigned int i = 0; i < 2; i++)
				{
					if (!m_vec_hist_data[i].Data.empty())
					{
						ImPlot::PlotLine(SENSING_FORCE_LAABLE[i], &m_vec_hist_data[i].Data[0].x, &m_vec_hist_data[i].Data[0].y,
							m_vec_hist_data[i].Data.size(), m_vec_hist_data[i].Offset, 2 * sizeof(float));
					}
				}
			}
			ImPlot::EndPlot();
		}
		ImGui::BulletText("Real time pose of the Fin-Ray.");
		//ImGui::SameLine();
		static float scale_factor = 15.1f;
		//ImGui::SliderFloat("force line", &scale_factor, 0.0f, 40.0f, "scale factor = %.1f");

		force_dirx[0] = force_pos[0];
		force_dirx[1] = force_pos[0] + force_dir[0] * fsign * scale_factor * 1e-3;
		force_diry[0] = force_pos[1];
		force_diry[1] = force_pos[1] + force_dir[1] * fsign * scale_factor * 1e-3;
		std::vector<double> shapedatax(m_calc_result.rxix, m_calc_result.rxix + sizeof(m_calc_result.rxix) / sizeof(double));
		std::vector<double> shapedatay(m_calc_result.rxiy, m_calc_result.rxiy + sizeof(m_calc_result.rxiy) / sizeof(double));
		std::vector<double> ribx(m_calc_result.ribx, m_calc_result.ribx + sizeof(m_calc_result.ribx) / sizeof(double));
		std::vector<double> riby(m_calc_result.riby, m_calc_result.riby + sizeof(m_calc_result.riby) / sizeof(double));

		this->UpdatePlotData("RightFinger", shapedatax, shapedatay,"rib", ribx, riby);


		const int points_num = 50;
		float object_x[points_num+1], object_y[points_num+1], center_point[2], obj_radius = 0.015;
		center_point[0] = contact_posx[0] - obj_radius * force_dir[0];
		center_point[1] = contact_posy[0] - obj_radius * force_dir[1];

		for (int i = 0; i < points_num+1; i++)
		{
			object_x[i] = center_point[0] + obj_radius * cos((i * 7.2) / 180 * pi);
			object_y[i] = center_point[1] + obj_radius * sin((i * 7.2) / 180 * pi);
		}
		static float line_weight = 4;
		static float marker_size = 6;
		static float marker_weight = 1;
		//ImGui::SliderFloat(u8"Line Width", &line_weight, 1.0f, 10.0f);
		//ImGui::SliderFloat(u8"Marker size", &marker_size, 1.0f, 10.0f);
		//ImGui::SliderFloat(u8"Marker weight", &marker_weight, 0.1f, 10.0f);

		if (ImPlot::BeginPlot(u8"Fin-Ray 位形", ImVec2(-FLT_MIN, -FLT_MIN), ImPlotFlags_Equal))
		{
			ImPlot::SetupAxes("x", "y", ImPlotAxisFlags_NoGridLines, ImPlotAxisFlags_NoGridLines);
			ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, line_weight);
			ImPlot::PushStyleColor(ImPlotCol_Line, (ImVec4)ImColor(255, 97, 0));
			for (auto& [k, v] : m_map_plot_shape_data)
			{
				ImPlot::PlotLine(k.c_str(), v.x.data(), v.y.data(), v.x.size());
			}
			ImPlot::PopStyleColor();
			ImPlot::PushStyleColor(ImPlotCol_Line, (ImVec4)ImColor(192,192,192));
			for (auto& [k, v] : m_map_plot_rib_data)
			{
				ImPlot::PlotLine(k.c_str(), v.rib1x.data(), v.rib1y.data(), v.rib1x.size());
				ImPlot::PlotLine(k.c_str(), v.rib2x.data(), v.rib2y.data(), v.rib2x.size());
				ImPlot::PlotLine(k.c_str(), v.rib3x.data(), v.rib3y.data(), v.rib3x.size());
				ImPlot::PlotLine(k.c_str(), v.rib4x.data(), v.rib4y.data(), v.rib4x.size());
				ImPlot::PlotLine(k.c_str(), v.rib5x.data(), v.rib5y.data(), v.rib5x.size());
			}
			ImPlot::PopStyleColor();
			ImPlot::PushStyleColor(ImPlotCol_Line, (ImVec4)ImColor(0, 255, 255));
			ImPlot::PlotLine("Contact object", object_x, object_y, 51);
			ImPlot::PopStyleColor();
			ImPlot::PushStyleColor(ImPlotCol_Line, (ImVec4)ImColor(255, 0, 0));
			ImPlot::PlotLine("Force diretion", force_dirx, force_diry, 2);
			ImPlot::PopStyleColor();
			ImPlot::PushStyleColor(ImPlotCol_Line, (ImVec4)ImColor(0, 255, 0));
			ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, marker_size, (ImVec4)ImColor(0, 255, 0), marker_weight, (ImVec4)ImColor(0, 0, 0));
			ImPlot::PlotLine("Contact position", contact_posx, contact_posy, 1);
			ImPlot::PopStyleColor();
			ImPlot::PopStyleVar(1);
			ImPlot::EndPlot();
		}
	}
	ImGui::End();
}

void UIFinRay::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case RRI_MSG_CALC_RESULT:
		OnFinRayCalcResult(from_id, data, msg_len);
		break;
	}
}

void UIFinRay::OnFinRayCalcResult(uint32_t from_id, const char* data, uint32_t msg_len)
{
	{
		std::lock_guard<std::mutex> lock(m_calc_result_lock);
		m_calc_result = *(ST_RRI_Calc_Result*)data;
	}
		char t6[64] = { 0 };
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		snprintf(t6, 64, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		strcpy(m_calc_result.t6, t6);

	{
		float t = (float)glfwGetTime();
	    initial_t = t;
		std::lock_guard<std::mutex> lock(m_vec_hist_data_lock);
		m_vec_hist_data[0].AddPoint(t, m_calc_result.contact_force);
		m_vec_hist_data[1].AddPoint(t, fd);
	}	

	if (m_calc_result.info.error_code != 0)
	{
		UI_WARN("FinRay Server Calc failed!");
	}
}

void UIFinRay::ExecuteForceFeedBackPolicy()
{
	UIMainWindow* main_win = dynamic_cast<UIMainWindow*>(m_main_win);
	if (m_main_win)
	{
		UIWindowManagerPtr mng = std::dynamic_pointer_cast<UIWindowManager>(main_win->GetWindowManager());
		if (mng)
		{
			UISensorMonitor* sensor = mng->GetSensorMonitor();

			ST_SensorData data = { 0 };
			sensor->GetCurrentSensorData(data);

			float t = (float)glfwGetTime();
			static float last_t = initial_t;
			float dt;
			dt = t - last_t;
			last_t = t;

			float ft;//sensing force by sensing algorithm
			float e, edot;
			static float eint = 0;
			static float last_e = 0;
			std::lock_guard<std::mutex> lock(m_calc_result_lock);


			ft = m_calc_result.contact_force;

			e = fd - ft;
			edot = (e - last_e) / dt;
			eint = eint + e * dt;
			
			last_e = e;
			
			float target_p;
			target_p = Kp * e + Ki * eint + Kd * edot;

			// map mean to motor speed
			UIEPOS2Control* epos2_control = mng->GetEPOS2Control();
			if (!m_epos2->IsOpen())
			{
				epos2_control->AttachEPOS2Ptr(m_epos_node_id - 1, m_epos2);

				if (m_epos2->OpenDevice())
				{
					UI_INFO(u8"打开Node [%d]成功！", m_epos_node_id);
				}
				else
				{
					UI_ERROR(u8"打开Node [%d]失败！", m_epos_node_id);
				}
				m_epos2->ClearFault();
				m_epos2->EnableNode();
			}

			char t7[64] = { 0 };
			SYSTEMTIME sys;
			GetLocalTime(&sys);
			snprintf(t7, 64, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
			strcpy(m_calc_result.t7, t7);
			if(is_start_record_time)
			{
				fp = fopen(filename, "a+");
				if (fp)
				{
					//std::lock_guard<std::mutex> lock(m_cur_data_lock);
					fprintf(fp, "%s,%s,%s,%s,%s,%s,%s", m_calc_result.t1, m_calc_result.t2, m_calc_result.t3, m_calc_result.t4, m_calc_result.t5, m_calc_result.t6, m_calc_result.t7);
					fprintf(fp, "\n");
				}
				if (fp)
				{
					fclose(fp);
				}
			}

			if (m_epos2->IsOpen())
			{
				//根据应变片示数进行软限位，读数绝对值超过9.5电机急停
				if (abs(ft)>7 || abs(data.data[0])>8.5 || data.data[1] > 8.5 || data.data[2] > 8.5 || data.data[3] > 8.5)
				{
					m_epos2->Halt();
				}
				else
				{
					float speed = target_p * 300;
					speed = speed > 6940 ? 6940 : speed;
					speed = speed < -6940 ? -6940 : speed;
					m_epos2->MoveV((long)speed);
				}
				//相对位置控制
				//float pos= -target_p * 4000;
				//pos = pos > 100000 ? 100000 : pos;
				//pos = pos < -100000 ? -100000 : pos;
				//m_epos2->MoveP((long)pos, false);
			}
		}
	}
}

void UIFinRay::SendCalcReq()
{
	if (!g_app.GetCPSApi()->IsDeviceOnline(g_cfg->m_finray_calc_server_id))
	{
		UI_WARN("Finray server not online!");
		return;
	}
	UIMainWindow* main_win = dynamic_cast<UIMainWindow*>(m_main_win);
	if (m_main_win)
	{
		UIWindowManagerPtr mng = std::dynamic_pointer_cast<UIWindowManager>(main_win->GetWindowManager());
		if (mng)
		{
			UISensorMonitor* sensor = mng->GetSensorMonitor();

			ST_SensorData data = { 0 };
			char send_t[64] = { 0 };
			
			//data.data[0] = -0.0900898166516817f;
			//data.data[1] = -0.137999540592015f;
			//data.data[2] = -0.193715738514712f;
			//data.data[3] = -0.128442744366743f;
			
			sensor->GetCurrentSensorData(data);			

			//data.data[3] = (data.data[0] * -2.5442 + 3.4852)/180*pi;
			//data.data[2] = (data.data[1] * -4.6684 + 6.9719)/180*pi;
			//data.data[1] = (data.data[2] * -4.8074 + 7.0618)/180*pi;
			//data.data[0] = (data.data[3] * -3.9238 + 5.5171)/180*pi;
			float ssdata1, ssdata2, ssdata3, ssdata4;

			ssdata1 = (data.data[0] * (float)calibration_parameter[0][0] + (float)calibration_parameter[0][1]) / 180 * pi;
			ssdata2 = (data.data[1] * (float)calibration_parameter[1][0] + (float)calibration_parameter[1][1]) / 180 * pi;
			ssdata3 = (data.data[2] * (float)calibration_parameter[2][0] + (float)calibration_parameter[2][1]) / 180 * pi;
			ssdata4 = (data.data[3] * (float)calibration_parameter[3][0] + (float)calibration_parameter[3][1]) / 180 * pi;

			data.data[3] = ssdata1;
			data.data[2] = ssdata2;
			data.data[1] = ssdata3;
			data.data[0] = ssdata4;

			SYSTEMTIME sys;
			GetLocalTime(&sys);
			snprintf(send_t, 64, "%02d:%02d:%02d.%03d", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
			strcpy(data.t3, send_t);
			g_app.GetCPSApi()->SendAPPMsg(g_cfg->m_finray_calc_server_id, MSG_SENSOR_DATA, (const char*)&data, sizeof(ST_SensorData));
		}
	}
}

std::vector<std::vector<double >> UIFinRay::readMatrixFile(const char *fileName)
{
	std::vector<std::vector<double>> matrixALL{};
    int row = 0;
 
    std::ifstream fileStream;
    std::string tmp;
    int count = 0;// 行数计数器
    fileStream.open(fileName, std::ios::in);//ios::in 表示以只读的方式读取文件
 
    if (fileStream.fail())//文件打开失败
    {
		char buf[1024] = { 0 };
		stbsp_sprintf(buf, u8"标定文件%s打开失败！", fileName);
		UI_WARN(buf);
    } else//文件存在
    {
        while (getline(fileStream, tmp, '\n'))//读取一行
        {
            //std::cout<<tmp<<std::endl;
            if(count==0)
			{
                row = std::stoi(tmp);
            }else
			{
                std::vector<double > tmpV{};
                std::istringstream is(tmp);
                for(int i=0;i<row;i++){
                    std::string str_tmp;
                    is>>str_tmp;
                   tmpV.push_back(std::stod(str_tmp));
                }
                matrixALL.push_back(tmpV);
            }
            count++;
        }
        fileStream.close();
    } 
    return matrixALL;
}