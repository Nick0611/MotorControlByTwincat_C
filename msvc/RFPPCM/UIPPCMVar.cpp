#include "UIPPCMVar.h"
#include "stb/stb_sprintf.h"
#include "imgui/imgui.h"
#include "IconsFontAwesome5.h"
#include "CPSSensorServerDef.h"
#include "UIWindowManager.h"
#include "UIMainWindow.h"
#include "UICfgParser.h"
//#include "GLFW/glfw3.h"


inline void SensorData2Angle(const ST_SensorData& data, double & r1, double & r2)
{
	if (data.sensor_num == 3)
	{
		r1 = -(data.data[1] - g_cfg->m_ppcm_cfg.encoder_init_pos[0]) / 4096 * 360 + 90;
		r2 = -(data.data[2] - g_cfg->m_ppcm_cfg.encoder_init_pos[1]) / 4096 * 360 + 90;
		// covert to radians
		r1 = r1 * pi / 180.0;
		r2 = r2 * pi / 180.0;
	}
}


UIPPCMVar::UIPPCMVar(UIGLWindow* main_win, const char* title): UIBaseWindow(main_win, title)
{

}

void UIPPCMVar::Draw()
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

	char buf[1024] = { 0 };

	stbsp_sprintf(buf, u8"%s 参数控制", ICON_FA_DESKTOP);
	if (ImGui::CollapsingHeader(buf, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (m_ppcm_control.IsConnected())
		{
			stbsp_sprintf(buf, u8"%s 断开控制器", ICON_FA_UNLINK);
			if (ImGui::Button(buf))
			{
				m_ppcm_control.DisconnectEPOS2();
			}
		}
		else
		{
			stbsp_sprintf(buf, u8"%s 连接控制器", ICON_FA_LINK);
			if (ImGui::Button(buf))
			{
				m_ppcm_control.ConnectEPOS2();
				UIMainWindow* main_win = dynamic_cast<UIMainWindow*>(m_main_win);
				if (m_main_win)
				{
					UIWindowManagerPtr mng = std::dynamic_pointer_cast<UIWindowManager>(main_win->GetWindowManager());
					if (mng)
					{
						for (size_t index = 0; index < m_ppcm_control.GetEPOS2Num(); index++)
						{
							mng->GetEPOS2Control()->AttachEPOS2Ptr(index, m_ppcm_control.GetEPOS2(index));
						}
					}
				}
			}
		}
		ImGui::SameLine();

		stbsp_sprintf(buf, u8"%s 清除错误", ICON_FA_TIMES);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.ClearFault();
		}
		ImGui::SameLine();

		stbsp_sprintf(buf, u8"%s 机构回零", ICON_FA_HOME);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.StartHoming();
		}
		ImGui::SameLine();

		stbsp_sprintf(buf, u8"%s 停止回零", ICON_FA_TIMES_CIRCLE);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.StopHoimg();
		}
		ImGui::SameLine();

		stbsp_sprintf(buf, u8"%s 使能", ICON_FA_PLUG);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.Enable();
		}
		ImGui::SameLine();

		stbsp_sprintf(buf, u8"%s 断开使能", ICON_FA_POWER_OFF);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.Disable();
		}
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
		stbsp_sprintf(buf, u8"%s 急停", ICON_FA_BAN);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.QuickStop();
		}
		ImGui::PopStyleColor(4);

		stbsp_sprintf(buf, u8"%s 电机输入", ICON_FA_EDIT);
		ImGui::Text(buf);
		{
			stbsp_sprintf(buf, u8"左侧R副-1[%.2f-%.2f]°", RFPPCM_JOINT_R_LOW_DEG, RFPPCM_JOINT_R_HIGH_DEG);
			ImGui::SliderFloat(buf, &m_motor_input[0], RFPPCM_JOINT_R_LOW_DEG, RFPPCM_JOINT_R_HIGH_DEG, u8"%.3f °");

			stbsp_sprintf(buf, u8"右侧R副-2[%.2f-%.2f]°", RFPPCM_JOINT_R_LOW_DEG, RFPPCM_JOINT_R_HIGH_DEG);
			ImGui::SliderFloat(buf, &m_motor_input[1], RFPPCM_JOINT_R_LOW_DEG, RFPPCM_JOINT_R_HIGH_DEG, u8"%.3f °");

			stbsp_sprintf(buf, u8"左侧P副-1[%.2f-%.2f]mm", RFPPCM_JOINT_P_LOW_MM, RFPPCM_JOINT_P_HIGH_MM);
			ImGui::SliderFloat(buf, &m_motor_input[2], RFPPCM_JOINT_P_LOW_MM, RFPPCM_JOINT_P_HIGH_MM, "%.3f mm");

			stbsp_sprintf(buf, u8"右侧P副-2[%.2f-%.2f]mm", RFPPCM_JOINT_P_LOW_MM, RFPPCM_JOINT_P_HIGH_MM);
			ImGui::SliderFloat(buf, &m_motor_input[3], RFPPCM_JOINT_P_LOW_MM, RFPPCM_JOINT_P_HIGH_MM, "%.3f mm");
		}

		stbsp_sprintf(buf, u8"%s 移动到指定位置", ICON_FA_ANGLE_DOUBLE_RIGHT);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.MoveTo(m_motor_input[0], m_motor_input[1], m_motor_input[2], m_motor_input[3]);
		}
		ImGui::SameLine();
		stbsp_sprintf(buf, u8"%s 停止移动", ICON_FA_TIMES_CIRCLE);
		if (ImGui::Button(buf))
		{
			m_ppcm_control.StopMove();
		}
		ImGui::SameLine();
		stbsp_sprintf(buf, u8"%s", m_ppcm_control.IsMoveEnd() ? u8"目标已达到" : u8"目标未达到");
		ImGui::InputText(u8"运动状态", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
	}
	stbsp_sprintf(buf, u8"%s 力感知", ICON_FA_DESKTOP);
	if (ImGui::CollapsingHeader(buf, ImGuiTreeNodeFlags_DefaultOpen))
	{
		static bool is_realtime =false;

		ImGui::Checkbox(u8"实时力感知", &is_realtime);
		if (is_realtime)
		{
			UpdateForceInfo();
		}

		stbsp_sprintf(buf, u8"%s 外力感知", ICON_FA_SATELLITE);
		ImGui::Text(buf);
		constexpr float FROCE_RANGE[2] = { -50, 50 };
		const char* FORCE_TITLE[2] = { "Fx", "Fy" };
		for (size_t i = 0; i < sizeof(m_force) / sizeof(float); i++)
		{
			float frac = (m_force[i] - FROCE_RANGE[0]) / (FROCE_RANGE[1] - FROCE_RANGE[0]);
			stbsp_sprintf(buf, u8"%.6f N", m_force[i]);
			ImGui::ProgressBar(frac, ImVec2(0.0f, 0.0f), buf);
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text(FORCE_TITLE[i]);
		}

		ImGui::Separator();
		stbsp_sprintf(buf, u8"%s 当前位形", ICON_FA_ANCHOR);
		ImGui::Text(buf);
		ImGui::InputFloat2("XY", m_pos, "%.6f", ImGuiInputTextFlags_ReadOnly);
	}
	ImGui::End();
}

void UIPPCMVar::UpdateForceInfo()
{
	UIMainWindow* main_win = dynamic_cast<UIMainWindow*>(m_main_win);
	if (!m_main_win)
	{
		return;
	}
	UIWindowManagerPtr mng = std::dynamic_pointer_cast<UIWindowManager>(main_win->GetWindowManager());
	if (!mng)
	{
		return;
	}
	ST_SensorData sensor = { 0 };
	//sensor.sensor_num = 3;
	mng->GetSensorMonitor()->GetCurrentSensorData(sensor);
	if (sensor.sensor_num <= 0)
	{
		return;
	}
	double sensor_rads[2] = { 0 };
	SensorData2Angle(sensor, sensor_rads[0], sensor_rads[1]);
	//sensor_rads[0] = 2.4312;
	//sensor_rads[1] = 0.8048;
	if (m_ppcm_control.GetEPOS2Num() == 4)
	{
		// 连杆参数
		Diamond_para param = { 0 };
		param.a = 0.16;
		param.b = 0.1;
		param.L1 = 0.114; //  0.12;
		param.L2 = 0.214; // 0.2;
		// 计算理论末端位置
		Vector2d xy = Diamond_forward(param, sensor_rads);
		m_pos[0] = (float)xy(0);
		m_pos[1] = (float)xy(1);

		// 力感知
		if (m_compute_future.valid())
		{
			std::future_status status = m_compute_future.wait_for(std::chrono::milliseconds(10));
			if (status != future_status::ready)
			{
				return;
			}

			ForceComputeResult result = m_compute_future.get();
			if (!result.result.success)
			{
				UI_ERROR(u8"求解失败！");
				return;
			}
			UI_INFO(u8"求解成功！");
			m_force[0] = (float)result.result.Fe(0);
			m_force[1] = (float)result.result.Fe(1);

			// 绘图
			MatrixXd L1, L2;
			plot_result(result.pcm_param, result.result.theta1, result.result.theta2, result.P, result.R, L1, L2);
			std::vector<double> v1x, v1y;
			std::vector<double> v2x, v2y;
			size_t v1size = L1.cols();
			for (size_t i = 0; i < v1size; i++)
			{
				v1x.push_back(L1(0, i));
				v1y.push_back(L1(1, i));
			}
			size_t v2size = L2.cols();
			for (size_t i = 0; i < v2size; i++)
			{
				v2x.push_back(L2(0, i));
				v2y.push_back(L2(1, i));
			}
			mng->GetPPCMPlot()->UpdatePlotData("L1", v1x, v1y);
			mng->GetPPCMPlot()->UpdatePlotData("L2", v2x, v2y);
		}
		else
		{					
			// 计算理论力
			int q1 = m_ppcm_control.GetMotorQC(0);
			int q2 = m_ppcm_control.GetMotorQC(1);
			int q3 = m_ppcm_control.GetMotorQC(2);
			int q4 = m_ppcm_control.GetMotorQC(3);
			double drive1_deg = Device::QC2Deg(q1);
			double drive2_deg = Device::QC2Deg(q2);
			double drive3_dist = DEG2MM(Device::QC2Deg(q3));
			double drive4_dist = DEG2MM(Device::QC2Deg(q4));

			Vector2d P, R;
			P << drive3_dist, drive4_dist;
			R << drive1_deg, drive2_deg; // pi / 3, -pi / 6; // 
			m_compute_future = std::async(std::launch::async, &UIPPCMVar::ComputeForce, this, xy, P, R);
		}
		//Force_perception_return result = Force_perception(pcm_param, xy(0), xy(1), P, R);
	}
}

ForceComputeResult UIPPCMVar::ComputeForce(Vector2d xy, Vector2d P, Vector2d R)
{
	// static params
	RFPCM_para pcm_param;
	pcm_param.distance = 0.1;
	pcm_param.Ki = 197e9;
	pcm_param.length_limb[0] = 0.23; pcm_param.length_limb[1] = 0.23;
	pcm_param.num_seg = 25;
	pcm_param.offset[0] = 0.05; pcm_param.offset[1] = 0.02;
	pcm_param.thickness = 0.5e-3;
	pcm_param.width = 15e-3;

	ForceComputeResult result;
	result.xy = xy;
	result.P = P;
	result.R = R;
	result.pcm_param = pcm_param;
	result.result = Force_perception(pcm_param, xy(0), xy(1), P, R);
	return result;
}
