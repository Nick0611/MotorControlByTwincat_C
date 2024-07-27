#include "UIEPOS2Control.h"
#include "imgui/imgui.h"
#include "IconsFontAwesome5.h"
#include "UILog.h"
#include "stb/stb_sprintf.h"


#define MAX_NODE_NUM	4

using namespace Device;

inline const char* State2Str(int state)
{
	switch (state)
	{
	case ST_DISABLED:
		return u8"�Ͽ�ʹ��";
	case ST_ENABLED:
		return u8"ʹ��";
	case ST_QUICKSTOP:
		return u8"��ͣ";
	case ST_FAULT:
		return u8"����";
	}
	return u8"δ֪";
}

inline const char* Mode2Str(char mode)
{
	switch (mode)
	{
	case OMD_PROFILE_POSITION_MODE:
		return u8"Profile Position ģʽ";
	case OMD_PROFILE_VELOCITY_MODE:
		return u8"Profile Velocity ģʽ";
	case OMD_HOMING_MODE:
		return u8"Homing ģʽ";
	case OMD_INTERPOLATED_POSITION_MODE:
		return u8"IPM ģʽ";
	case OMD_POSITION_MODE:
		return u8"Position ģʽ";
	case OMD_VELOCITY_MODE:
		return u8"Velocity ģʽ";
	case OMD_CURRENT_MODE:
		return u8"Current ģʽ";
	case OMD_MASTER_ENCODER_MODE:
		return u8"Master Encoder ģʽ";
	case OMD_STEP_DIRECTION_MODE:
		return u8"Step Direction ģʽ";
	}
	return u8"δ֪";
}

UIEPOS2Control::UIEPOS2Control(UIGLWindow* main_win, const char* title):UIBaseWindow(main_win, title)
{
	for (int i = 0; i < MAX_NODE_NUM; i++)
	{
		m_vec_epos2.push_back(std::make_shared<Device::EPOS2>(i+1));
	}
	DWORD err_code;
	VCS_GetDriverInfo(m_epos_sdk_name, sizeof(m_epos_sdk_name), 
		m_epos_sdk_ver, sizeof(m_epos_sdk_ver), &err_code);
}

void UIEPOS2Control::AttachEPOS2Ptr(size_t index, std::shared_ptr<Device::EPOS2> epos2)
{
	if (!epos2)
	{
		UI_ERROR(u8"%sʧ��, ��Ч��EPOS2����", __FUNCTION__);
	}

	if (index >= 0 && index < MAX_NODE_NUM)
	{
		m_vec_epos2[index] = epos2;
	}
}

std::shared_ptr<Device::EPOS2> UIEPOS2Control::GetEPOS2Ptr(size_t index)
{
	if (index >= 0 && index < MAX_NODE_NUM)
	{
		return m_vec_epos2[index];
	}
	return nullptr;
}

void UIEPOS2Control::Draw()
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
	char buf[256] = { 0 };
	
	if (ImGui::CollapsingHeader(u8"������Ϣ", ImGuiTreeNodeFlags_DefaultOpen))
	{
		stbsp_sprintf(buf, u8"%s EPOS SDK��Ϣ", ICON_FA_INFO_CIRCLE);
		ImGui::Text(buf);

		ImGui::InputText("Name", m_epos_sdk_name, sizeof(m_epos_sdk_name), ImGuiInputTextFlags_ReadOnly);
		ImGui::SameLine();
		ImGui::InputText("Version", m_epos_sdk_ver, sizeof(m_epos_sdk_ver), ImGuiInputTextFlags_ReadOnly);

		stbsp_sprintf(buf, u8"%s ѡ��Node ID", ICON_FA_LIST);
		ImGui::Text(buf);

		char preview_value[64] = { 0 };
		if (m_selected_index >= 0)
		{
			snprintf(preview_value, sizeof(preview_value), "%s Node [%d]", ICON_FA_MICROCHIP, m_selected_index + 1);
		}
		if (ImGui::BeginCombo(u8"Node ID�б�", preview_value, 0))
		{
			for (int i = 0; i < MAX_NODE_NUM; i++)
			{
				stbsp_sprintf(buf, u8"%s Node [%d]", ICON_FA_MICROCHIP, i + 1);
				if (ImGui::Selectable(buf, m_selected_index == i))
				{
					m_selected_index = i;
				}
			}
			ImGui::EndCombo();
		}

		if (m_selected_index >= 0)
		{
			ImGui::SameLine();

			auto& epos2 = m_vec_epos2[m_selected_index];
			if (!epos2->IsOpen())
			{
				stbsp_sprintf(buf, u8"%s ����", ICON_FA_LINK);
				if (ImGui::Button(buf))
				{
					if (epos2->OpenDevice())
					{
						UI_INFO(u8"��Node [%d]�ɹ���", m_selected_index + 1);
					}
					else
					{
						UI_ERROR(u8"��Node [%d]ʧ�ܣ�", m_selected_index + 1);
					}
				}
			}
			else
			{
				stbsp_sprintf(buf, u8"%s �Ͽ�", ICON_FA_UNLINK);
				if (ImGui::Button(buf))
				{
					if (epos2->CloseDevice())
					{
						UI_INFO(u8"�Ͽ�Node [%d]�ɹ���", m_selected_index + 1);
					}
					else
					{
						UI_ERROR(u8"�Ͽ�Node [%d]ʧ�ܣ�", m_selected_index + 1);
					}
				}
			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s ȫ���Ͽ�", ICON_FA_UNLINK);
			if (ImGui::Button(buf))
			{
				for (auto p: m_vec_epos2)
				{
					p->CloseDevice();
				}
			}
		}
	}
	if (m_selected_index >= 0)
	{
		std::shared_ptr<Device::EPOS2> & epos2 = m_vec_epos2[m_selected_index];
		if (ImGui::CollapsingHeader(u8"�������汾", ImGuiTreeNodeFlags_DefaultOpen))
		{
			stbsp_sprintf(buf, "%d", epos2->m_hardwareVersion);
			ImGui::InputText(u8"Ӳ���汾", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			stbsp_sprintf(buf, "%d", epos2->m_softwareVersion);
			ImGui::InputText(u8"����汾", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

			stbsp_sprintf(buf, "%d", epos2->m_applicationNumber);
			ImGui::InputText(u8"������", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			stbsp_sprintf(buf, "%d", epos2->m_applicationVersion);
			ImGui::InputText(u8"����汾", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
		}
		if (ImGui::CollapsingHeader(u8"��������", ImGuiTreeNodeFlags_DefaultOpen))
		{
			stbsp_sprintf(buf, u8"%s ����", ICON_FA_DOWNLOAD);
			if (ImGui::Button(buf))
			{
				UI_INFO(u8"���ڶ�ȡ����������...");
				int rc = epos2->ReadNodeParams();
				UI_INFO(u8"��ȡ����������%s[%d].", rc ? u8"�ɹ�" : u8"ʧ��", rc);
			}
			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s ����", ICON_FA_UPLOAD);
			if (ImGui::Button(buf))
			{
				UI_INFO(u8"����д�����������..."); 
				int rc = epos2->WriteNodeParams();
				UI_INFO(u8"д�����������%s[%d].", rc ? u8"�ɹ�" : u8"ʧ��", rc);
			}
			if (ImGui::TreeNodeEx(u8"��ȫ����", ImGuiTreeNodeFlags_DefaultOpen))
			{
				EPOS2SafetyParam* p = epos2->GetSafetyInfo();
				ImGui::InputScalar("MaxFollowingError  ", ImGuiDataType_U32, (void*)&p->MaxFollowingError);
				ImGui::SameLine();
				ImGui::InputScalar("MaxAcceleration", ImGuiDataType_U32, (void*)&p->MaxAcceleration);
				ImGui::InputScalar("MaxProfileVelocity ", ImGuiDataType_U32, (void*)&p->MaxProfileVelocity);
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx(u8"λ���ƶ�����", ImGuiTreeNodeFlags_DefaultOpen))
			{
				PositionProfileParam* p = epos2->GetPositionProfileParam();
				ImGui::InputScalar("ProfileAcceleration", ImGuiDataType_U32, (void*)&p->ProfileAcceleration);
				ImGui::SameLine();
				ImGui::InputScalar("ProfileDeceleration", ImGuiDataType_U32, (void*)&p->ProfileDeceleration);
				ImGui::InputScalar("ProfileVelocity    ", ImGuiDataType_U32, (void*)&p->ProfileVelocity);
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx(u8"�ٶ��ƶ�����", ImGuiTreeNodeFlags_DefaultOpen))
			{
				VelocityProfileParam* p = epos2->GetVelocityProfileParam();
				ImGui::InputScalar("ProfileAcceleration", ImGuiDataType_U32, (void*)&p->ProfileAcceleration);
				ImGui::SameLine();
				ImGui::InputScalar("ProfileDeceleration", ImGuiDataType_U32, (void*)&p->ProfileDeceleration);
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx(u8"IPM����", ImGuiTreeNodeFlags_DefaultOpen))
			{
				IPMParam* p = epos2->GetIPMParam();
				ImGui::InputScalar("UnderFlowWarningLimit", ImGuiDataType_U16, (void*)&p->UnderFlowWarningLimit);
				ImGui::SameLine();
				ImGui::InputScalar("OverFlowWarningLimit", ImGuiDataType_U16, (void*)&p->OverFlowWarningLimit);
				ImGui::InputScalar("MaxBufferSize", ImGuiDataType_U32, (void*)&p->MaxBufferSize, NULL, NULL, NULL, ImGuiInputTextFlags_ReadOnly);
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx(u8"Home����", ImGuiTreeNodeFlags_DefaultOpen))
			{
				HomingParam* p = epos2->GetHomingParam();
				ImGui::InputScalar("HomingAcceleration ", ImGuiDataType_U32, (void*)&p->HomingAcceleration);
				ImGui::SameLine();
				ImGui::InputScalar("SpeedSwitch", ImGuiDataType_U32, (void*)&p->SpeedSwitch);
				ImGui::InputScalar("SpeedIndex         ", ImGuiDataType_U32, (void*)&p->SpeedIndex);
				ImGui::SameLine();
				ImGui::InputScalar("HomeOffset", ImGuiDataType_U32, (void*)&p->HomeOffset);
				ImGui::InputScalar("CurrentTreshold    ", ImGuiDataType_U16, (void*)&p->CurrentTreshold);
				ImGui::SameLine();
				ImGui::InputScalar("HomePosition", ImGuiDataType_U32, (void*)&p->HomePosition);
				ImGui::TreePop();
			}
		}
		if (ImGui::CollapsingHeader(u8"�������", ImGuiTreeNodeFlags_DefaultOpen))
		{
			stbsp_sprintf(buf, u8"%s �������", ICON_FA_TIMES);
			if (ImGui::Button(buf))
			{
				epos2->ClearFault();
			}
			ImGui::SameLine();

			stbsp_sprintf(buf, u8"%s ʹ��", ICON_FA_PLUG);
			if (ImGui::Button(buf))
			{
				epos2->EnableNode();
			}
			ImGui::SameLine();

			stbsp_sprintf(buf, u8"%s �Ͽ�ʹ��", ICON_FA_POWER_OFF);
			if (ImGui::Button(buf))
			{
				epos2->DisableNode();
			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s �������λ", ICON_FA_HOME);
			if (ImGui::Button(buf))
			{
				epos2->FindHomeByPosLimit();
			}

			ImGui::SameLine();

			stbsp_sprintf(buf, u8"%s �������λ", ICON_FA_HOME);
			if (ImGui::Button(buf))
			{
				epos2->FindHomeByNegLimit();
			}

			ImGui::SameLine();
			stbsp_sprintf(buf, u8"%s ֹͣ����", ICON_FA_TIMES_CIRCLE);
			if (ImGui::Button(buf))
			{
				epos2->StopHoming();
			}

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.9f, 0.9f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
			stbsp_sprintf(buf, u8"%s ��ͣ", ICON_FA_BAN);
			if (ImGui::Button(buf))
			{
				epos2->QuickStop();
			}
			ImGui::PopStyleColor(4);

			static int qc = 0;
			ImGui::InputInt(u8"λ�ÿ���", &qc, 1000, 10000);
			ImGui::SameLine();

			stbsp_sprintf(buf, u8"�ƶ���λ��");
			if (ImGui::Button(buf))
			{
				epos2->MoveP(qc);
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"��ͣ�ƶ�##1"))
			{
				epos2->Halt();
			}

			static int speed = 0;
			ImGui::InputInt(u8"�ٶȿ���", &speed, 1000, 10000);
			ImGui::SameLine();

			stbsp_sprintf(buf, u8"���ٶ��ƶ�");
			if (ImGui::Button(buf))
			{
				epos2->MoveV(speed);
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"��ͣ�ƶ�##2"))
			{
				epos2->Halt();
			}

			if (ImGui::TreeNodeEx(u8"IPMģʽ", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button(u8"���ع켣����..."))
				{
					epos2->ClearPVTQuque();
					stbsp_sprintf(buf, "node_%d.txt", (int)epos2->GetNodeID());
					epos2->LoadPVTData(buf);
				}
				ImGui::SameLine();
				if (ImGui::Button(u8"��ʼ�˶�"))
				{
					epos2->StartIPMMode();
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					epos2->StartTrajectory();
				}
				ImGui::SameLine();
				if (ImGui::Button(u8"ֹͣ�˶�"))
				{
					epos2->StopTrajectory();
					epos2->StopIPMMode();
				}
				ImGui::TreePop();
			}
		}
		if (ImGui::CollapsingHeader(u8"������", ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::string state_str = State2Str(epos2->GetState());
			ImGui::InputText(u8"״̬", (char*)state_str.c_str(), state_str.size(), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			std::string mode_str = Mode2Str(epos2->GetOperationMode());
			ImGui::InputText(u8"ģʽ", (char*)mode_str.c_str(), mode_str.size(), ImGuiInputTextFlags_ReadOnly);

			stbsp_sprintf(buf, "%d", epos2->GetPosition());
			ImGui::InputText(u8"λ��", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			stbsp_sprintf(buf, "%d", epos2->GetTargetPosition());
			ImGui::InputText(u8"Ŀ��λ��", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

			stbsp_sprintf(buf, "%d", epos2->GetVelocity());
			ImGui::InputText(u8"�ٶ�", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
			ImGui::SameLine();
			stbsp_sprintf(buf, "%d", epos2->GetTargetVelocity());
			ImGui::InputText(u8"Ŀ���ٶ�", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

			stbsp_sprintf(buf, u8"%s", epos2->GetMoveState() ? u8"Ŀ���Ѵﵽ" : u8"Ŀ��δ�ﵽ");
			ImGui::InputText(u8"�˶�״̬", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

			if (ImGui::TreeNodeEx(u8"����IO", ImGuiTreeNodeFlags_DefaultOpen))
			{
				DigitalInputs di = { 0 };
				epos2->GetDigitalInputs(di);
				ImGui::RadioButton("Home Switch[4]", di.DI_HOME_SWITCH ? true : false);
				ImGui::SameLine();
				ImGui::RadioButton("Positive Limit[5]", di.DI_POSITIVE_LIMIT_SWITCH ? true : false);
				ImGui::SameLine();
				ImGui::RadioButton("Negative Limit[6]", di.DI_NEGATIVE_LIMIT_SWITCH ? true : false);

				ImGui::TreePop();
			}
		}
	}

	ImGui::End();
}
