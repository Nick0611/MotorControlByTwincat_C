#include "UISysSettings.h"
#include "UIMainWindow.h"
#include "stb/stb_sprintf.h"
#include "IconsFontAwesome5.h"

UISysSettings::UISysSettings(UIGLWindow* main_win, const char* title) :UIBaseWindow(main_win, title)
{
	m_main_win = dynamic_cast<UIMainWindow*>(main_win);
}

void UISysSettings::Draw()
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
	if (ImGui::CollapsingHeader(u8"窗口设置", ImGuiTreeNodeFlags_DefaultOpen))
	{
		stbsp_sprintf(buf, u8"%s 设置窗口背景色", ICON_FA_PALETTE);
		ImGui::Text(buf);
		UIStyle* uistyle = m_main_win->CurrentStyle();

		if (ImGui::ColorEdit3(u8"背景色", (float*)&uistyle->bk_color1)) // Edit 3 floats representing a color
		{
		}

		stbsp_sprintf(buf, u8"%s 选择界面主题", ICON_FA_COOKIE);
		ImGui::Text(buf);
		static int style_idx = 3;
		if (ImGui::Combo(u8"主题", &style_idx, "Dark\0DarkLight\0Solar\0Blue\0"))
		{
			switch (style_idx)
			{
			case 0: m_main_win->SetUIStyle("Dark"); break;
			case 1: m_main_win->SetUIStyle("DarkLight"); break;
			case 2: m_main_win->SetUIStyle("Solar"); break;
			case 3: m_main_win->SetUIStyle("Blue"); break;
			}
		}
		stbsp_sprintf(buf, u8"%s 设置界面透明度", ICON_FA_ADJUST);
		ImGui::Text(buf);
		ImGuiStyle& style = ImGui::GetStyle();
		ImGui::SliderFloat(u8"透明度", &style.Colors[ImGuiCol_WindowBg].w, 0.0f, 1.0f, "%.1f");

		stbsp_sprintf(buf, u8"%s 设置UI字体缩放", ICON_FA_ARROWS_ALT_H);
		ImGui::Text(buf);
		if (ImGui::SliderFloat(u8"字体缩放", &ImGui::GetIO().FontGlobalScale, 1.0f, 3.0f, "%.1f"))  // Edit 1 float using a slider from 0.0f to 1.0f
		{
		}

		stbsp_sprintf(buf, u8"%s 设置菜单显示", ICON_FA_BARS);
		ImGui::Text(buf);
		bool show_menu = m_main_win->IsMainMenuVisible();
		if (ImGui::Checkbox(u8"显示主菜单", &show_menu))
		{
			m_main_win->SetMainMenuVisible(show_menu);
		}
	}
	if (ImGui::CollapsingHeader(u8"高级设置", ImGuiTreeNodeFlags_DefaultOpen))
	{
		stbsp_sprintf(buf, u8"%s 设置OpenGL模式", ICON_FA_SLIDERS_H);
		ImGui::Text(buf);
		int mode = (int)m_main_win->GetUIMode();
		if (ImGui::Combo(u8"模式", &mode, u8"性能模式\0平衡模式\0固定模式\0"))
		{
			switch (mode)
			{
			case 0: m_main_win->SetUIMode(E_UI_POLL_MODE); break;
			case 1: m_main_win->SetUIMode(E_UI_WAIT_MODE); break;
			case 2: m_main_win->SetUIMode(E_UI_WAIT_TIMEOUT_MODE); break;
			}
		}
		if (mode == int(E_UI_WAIT_TIMEOUT_MODE))
		{
			float timeout = (float)m_main_win->GetUIRefreshTimeout();
			if (ImGui::SliderFloat(u8"刷新间隔", &timeout, 0.001f, 1.0f, "%.3f"))
			{
				m_main_win->SetUIRefreshTimeout((double)timeout);
			}
		}
	}
	ImGui::Separator();
	ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), u8"FPS %.3f ms/frame (%.1f FPS)",
		1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}
