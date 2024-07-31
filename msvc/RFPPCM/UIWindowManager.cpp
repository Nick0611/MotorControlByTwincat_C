#include "UIWindowManager.h"
#include "UIMainWindow.h"
#include "UIApplication.h"
#include "IconsFontAwesome5.h"
#include "UIFontLoader.h"
#include "UITextureLoader.h"
#include "UIHelpAbout.h"
#include "UIHelpDoc.h"

#include "ImGuiFileDialog/ImGuiFileDialog.h"
#include "ImGuiFileDialog/CustomFont.h"

#include "imgui/imgui_internal.h"
#ifdef UI_CFG_USE_IMPLOT
#include "implot/implot.h"
#endif

UIWindowManager::UIWindowManager(UIMainWindow* ui_main):
	UIWindowManagerBase(ui_main)
{

}

UIWindowManager::~UIWindowManager()
{

}


void UIWindowManager::Init()
{
	// init imdialog
	InitImDialog();

	// create ui windows
	InitWindows();
	
	// init menus
	InitMenus();
}

void UIWindowManager::Draw()
{
	ShowDemo();
#ifdef UI_CFG_USE_IMPLOT
	ShowPlotDemo();
#endif

	for (size_t i = 0; i < m_win_list.size(); i++)
	{
		m_win_list[i]->Draw();
	}

	// set initial layout
	if (m_reset_layout)
	{
		ResetLayout();
		m_reset_layout = false;
	}
	// display
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(480, 320)))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			// action
			OpenFile(filePathName);
		}
		// close
		ImGuiFileDialog::Instance()->Close();
	}
	// save 
	if (ImGuiFileDialog::Instance()->Display("SaveFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(480, 320)))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
			// action
			SaveFile(filePathName);
		}
		// close
		ImGuiFileDialog::Instance()->Close();
	}
}

void UIWindowManager::InitWindows()
{
	char buf[256] = { 0 };

	sprintf(buf, u8"%s 系统设置", ICON_FA_COGS);
	m_ui_sys = new UISysSettings(m_ui_main, buf);
	m_win_list.push_back(m_ui_sys);

	sprintf(buf, u8"%s 图像", ICON_FA_IMAGE);
	m_ui_image = new UIImageView(m_ui_main, buf);
	m_win_list.push_back(m_ui_image);

	sprintf(buf, u8"%s 日志", ICON_FA_PRINT);
	m_ui_log = new UILogView(m_ui_main, buf);
	m_win_list.push_back(m_ui_log);
#ifdef UI_CFG_BUILD_DOC
	sprintf(buf, u8"%s 使用文档", ICON_FA_BOOK);
	m_win_list.push_back(new UIHelpDoc(m_ui_main, buf));
#endif
	sprintf(buf, u8"%s 关于", ICON_FA_INFO);
	m_win_list.push_back(new UIHelpAbout(m_ui_main, buf));

	//sprintf(buf, u8"%s PPCM控制", ICON_FA_ATOM);
	//m_ui_var = new UIPPCMVar(m_ui_main, buf);
	//m_win_list.push_back(m_ui_var);

	//sprintf(buf, u8"%s PPCM Plot", ICON_FA_DRAW_POLYGON);
	//m_ui_plot = new UIPPCMPlot(m_ui_main, buf);
	//m_win_list.push_back(m_ui_plot);

	sprintf(buf, u8"%s OptiTrack Monitor", ICON_FA_MAP_MARKER_ALT);
	m_ui_opt = new UIOptiTrack(m_ui_main, buf);
	m_win_list.push_back(m_ui_opt); 
	
	sprintf(buf, u8"%s Sensor Monitor", ICON_FA_MICROCHIP);
	m_ui_sensor = new UISensorMonitor(m_ui_main, buf);
	m_win_list.push_back(m_ui_sensor);

	sprintf(buf, u8"%s EPOS2 Controller", ICON_FA_ATOM);
	m_ui_epos2 = new UIEPOS2Control(m_ui_main, buf);
	m_win_list.push_back(m_ui_epos2);

	sprintf(buf, u8"%s Hypersen Monitor", ICON_FA_ARROWS_ALT);
	m_ui_hypersen = new UIHypersenMonitor(m_ui_main, buf);
	m_win_list.push_back(m_ui_hypersen);

	sprintf(buf, u8"%s FinRay", ICON_FA_ALLERGIES);
	m_ui_finray = new UIFinRay(m_ui_main, buf);
	m_win_list.push_back(m_ui_finray);

	sprintf(buf, u8"%s Trolley Controller", ICON_FA_BACON);
	m_ui_trolley = new UITrolleyControlTest(m_ui_main, buf);
	m_win_list.push_back(m_ui_trolley);

	sprintf(buf, u8"%s HUATRAN Force Sensor", ICON_FA_BAN);
	m_ui_force_huatran = new UIForceSensor_HUATRAN(m_ui_main, buf);
	m_win_list.push_back(m_ui_force_huatran);
}

void UIWindowManager::InitMenus()
{
	// 创建主菜单
	{
		UIMainMenu m;
		sprintf(m.name, u8"%s 系统", ICON_FA_TOOLS);
		m.cat = EUIMenuCategory::E_UI_CAT_SYS;
		m.show = true;
		m_ui_main->AddMainMenu(m);

		sprintf(m.name, u8"%s 编辑", ICON_FA_EDIT);
		m.cat = EUIMenuCategory::E_UI_CAT_EDIT;
		m.show = false;
		m_ui_main->AddMainMenu(m);

		sprintf(m.name, u8"%s 显示", ICON_FA_EYE);
		m.cat = EUIMenuCategory::E_UI_CAT_VIEW;
		m.show = true;
		m_ui_main->AddMainMenu(m);

		sprintf(m.name, u8"%s 应用", ICON_FA_ROCKET);
		m.cat = EUIMenuCategory::E_UI_CAT_APP;
		m.show = true;
		m_ui_main->AddMainMenu(m);

		sprintf(m.name, u8"%s 设备", ICON_FA_SERVER);
		m.cat = EUIMenuCategory::E_UI_CAT_SERVICE;
		m.show = false;
		m_ui_main->AddMainMenu(m);

		sprintf(m.name, u8"%s 工具", ICON_FA_TOOLBOX);
		m.cat = EUIMenuCategory::E_UI_CAT_TOOL;
		m.show = true;
		m_ui_main->AddMainMenu(m);

		sprintf(m.name, u8"%s 窗口", ICON_FA_TH_LARGE);
		m.cat = EUIMenuCategory::E_UI_CAT_LAYOUT;
		m.show = true;
		m_ui_main->AddMainMenu(m);

		sprintf(m.name, u8"%s 帮助", ICON_FA_QUESTION_CIRCLE);
		m.cat = EUIMenuCategory::E_UI_CAT_HELP;
		m.show = true;
		m_ui_main->AddMainMenu(m);
	}

	{
		char buf[256] = { 0 };
		// 1. 窗口菜单前面
		sprintf(buf, u8"%s 打开...", ICON_FA_FOLDER_OPEN);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_SYS, [&]() {
			this->ShowOpenModelFileDialog();
		}, buf, "Ctrl+O"));

		sprintf(buf, u8"%s 导出...", ICON_FA_FOLDER_OPEN);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_SYS, [&]() {
			this->ShowSaveModelFileDialog();
		}, buf, "Ctrl+P"));
		// 2. 窗口菜单
		std::for_each(m_win_list.begin(), m_win_list.end(), [&](UIBaseWindow* win) {
			UIWindowMenuPtr m = std::make_shared<UIWindowMenu>(win);
			m_ui_main->AddMenuItem(m);
		});

		//3. 窗口菜单之后
		sprintf(buf, u8"%s 退出", ICON_FA_SIGN_OUT_ALT);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_SYS, [&]() {
			m_ui_main->SignalClose();
		}, buf, "Esc"));

#ifdef _DEBUG
		sprintf(buf, u8"%s ImGUI Demo", ICON_FA_ICONS);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_VIEW, [&]() {
			this->SetShowDemo(true);
		}, buf, nullptr));
#ifdef UI_CFG_USE_IMPLOT
		sprintf(buf, u8"%s ImPlot Demo", ICON_FA_DRAW_POLYGON);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_VIEW, [&]() {
			this->SetShowPlotDemo(true);
		}, buf, nullptr));
#endif
#endif
		sprintf(buf, u8"%s 全屏", ICON_FA_EXPAND);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_VIEW, [&]() {
			m_ui_main->ToggleFullscreen();
		}, buf, "Ctrl+Shift+F"));

		sprintf(buf, u8"%s 隐藏所有窗口", ICON_FA_WINDOW_MINIMIZE);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_LAYOUT, [&]() {
			this->HideAllWindow();
		}, buf, "Ctrl+Shift+H"));

		sprintf(buf, u8"%s 显示所有窗口", ICON_FA_WINDOW_MAXIMIZE);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_LAYOUT, [&]() {
			this->ShowAllWindow();
		}, buf, "Ctrl+Shift+D"));

		sprintf(buf, u8"%s 重置窗口布局", ICON_FA_WINDOW_RESTORE);
		m_ui_main->AddMenuItem(std::make_shared<UIFunctionMenu>(
			EUIMenuCategory::E_UI_CAT_LAYOUT, [&]() {
			this->SetResetLayoutFlag();
		}, buf, "Ctrl+Shift+R"));
	}
}

void UIWindowManager::InitImDialog()
{
	// ImGuiFileDialog settings
	// define style by file extention and Add an icon for .png files 
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".jpg", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE);
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), ICON_IGFD_FILE);
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".bmp", ImVec4(1.0f, 1.0f, 0.5f, 0.9f), ICON_IGFD_FILE);
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(1.0f, 0.5f, 0.5f, 0.9f), ICON_IGFD_FILE);

	// define style for all directories
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);
	// can be for a specific directory
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, ".git", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);

	// define style for all files
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FILE);
	// can be for a specific file
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, ".git", ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FILE);
}

void UIWindowManager::ResetLayout()
{
#ifdef IMGUI_HAS_DOCK
	ImGuiID dock_id = m_ui_main->GetRootDockSpaceID();

	ImGui::DockBuilderRemoveNode(dock_id); // Clear out existing layout
	ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->WorkSize);

	ImGuiID dock_main_id = dock_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
	ImGuiID dock_id_log = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
	//ImGuiID dock_id_var = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.60f, NULL, &dock_main_id);

	// set node local flags
	//ImGuiDockNode* node_log = ImGui::DockBuilderGetNode(dock_id_log);
	//node_log->SetLocalFlags(ImGuiDockNodeFlags_NoTabBar);

	//m_ui_var->Show();
	//m_ui_plot->Show();
	m_ui_log->Show();
	m_ui_opt->Show();
	m_ui_sensor->Show();
	m_ui_epos2->Show();
	m_ui_trolley->Show();

	//ImGui::DockBuilderDockWindow(m_ui_var->GetWinTitle(), dock_id_var);
	//ImGui::DockBuilderDockWindow(m_ui_plot->GetWinTitle(), dock_main_id);
	ImGui::DockBuilderDockWindow(m_ui_opt->GetWinTitle(), dock_main_id);
	ImGui::DockBuilderDockWindow(m_ui_sensor->GetWinTitle(), dock_main_id);
	ImGui::DockBuilderDockWindow(m_ui_epos2->GetWinTitle(), dock_main_id);
	ImGui::DockBuilderDockWindow(m_ui_trolley->GetWinTitle(), dock_main_id);
	ImGui::DockBuilderDockWindow(m_ui_log->GetWinTitle(), dock_id_log);

	ImGui::DockBuilderFinish(dock_id);
#endif
}

void UIWindowManager::OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	// forward cps message to every ui
	for (size_t i = 0; i < m_win_list.size(); i++)
	{
		m_win_list[i]->OnCPSMsg(from_id, msg_type, data, msg_len);
	}
}

void UIWindowManager::Destroy()
{
	// release window ptr
	for (size_t i = 0; i < m_win_list.size(); i++)
	{
		delete m_win_list[i];
	}
	m_win_list.clear();
}

void UIWindowManager::ShowOpenModelFileDialog()
{
	const char* filters = "All files{.jpg,.png,.bmp,.gif,},.jpg,.png,.bmp,.gif,";
	char buf[256] = { 0 };
	sprintf(buf, u8"%s 打开文件", ICON_IGFD_FOLDER_OPEN);
	ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", buf, filters, ".", 1, nullptr, ImGuiFileDialogFlags_CaseInsensitiveExtention);
}

void UIWindowManager::ShowSaveModelFileDialog()
{
	const char* filters = "All files{.jpg,.png},.jpg,.png";
	char buf[256] = { 0 };
	sprintf(buf, u8"%s 导出", ICON_IGFD_SAVE);
	ImGuiFileDialog::Instance()->OpenModal("SaveFileDlgKey", buf, filters, ".", 1, nullptr, ImGuiFileDialogFlags_CaseInsensitiveExtention);
}

void UIWindowManager::OpenFile(const std::string& filename)
{
	UI_INFO(u8"Opening file %s ...", filename.c_str());
	m_ui_image->ShowImageFromFile(filename.c_str());
}

void UIWindowManager::SaveFile(const std::string& filename)
{
	UI_INFO(u8"export to file %s ...", filename.c_str());
}

void UIWindowManager::ShowAllWindow()
{
	m_show_demo = true;
	m_show_plot_demo = true;

	for (size_t i = 0; i < m_win_list.size(); i++)
	{
		// 不显示帮助窗口
		if (m_win_list[i]->GetWinMenuCategory() != EUIMenuCategory::E_UI_CAT_HELP)
		{
			m_win_list[i]->Show();
		}
	}
}

void UIWindowManager::HideAllWindow()
{
	m_show_demo = false;
	m_show_plot_demo = false;

	for (size_t i = 0; i < m_win_list.size(); i++)
	{
		m_win_list[i]->Hide();
	}
}

void UIWindowManager::ShowDemo()
{
	if (!m_show_demo)
	{
		return;
	}
#ifdef _DEBUG
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	ImGui::ShowDemoWindow(&m_show_demo);
#endif
}

#ifdef UI_CFG_USE_IMPLOT
void UIWindowManager::ShowPlotDemo()
{
	if (!m_show_plot_demo)
	{
		return;
	}
#ifdef _DEBUG
	ImPlot::ShowDemoWindow(&m_show_plot_demo);
#endif
}
#endif