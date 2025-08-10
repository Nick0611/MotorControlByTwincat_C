#pragma once
#include <vector>
#include <map>
#include <string>
#include "imgui/imgui.h"
#include "UIBaseWindow.h"
#include "UILog.h"
#include "UIGLWindow.h"
#include "UIDef.h"
#include "UIWindowManagerBase.h"
#include "UIMenu.h"

struct UIStyle
{
	ImGuiStyle	imstyle;
	ImVec4		bk_color1;
	ImVec4		bk_color2;
	int			fill_style; // Aspect_GradientFillMethod
};

class UIMainWindow : public UIGLWindow
{
	friend UIBaseWindow;
public:
	UIMainWindow();
	virtual ~UIMainWindow();

	/* 初始化窗口系统 
	* title: window title
	* flags: UIGLWindowFlags
	*/
	void Init(const char* title, int flags);
	// 进入窗口循环
	void Loop();

	// 处理 cps message
	void OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len);
	// 销毁窗口（程序退出时调用）
	void Destroy();

	bool IsClosing() { return m_close; }
	bool IsInitialized() { return m_is_initialized; }

	void SetWindowManager(UIWindowManagerBasePtr win_mng) { m_win_mng = win_mng; }
	UIWindowManagerBasePtr GetWindowManager() { return m_win_mng; }
	ImGuiID GetRootDockSpaceID() { return m_dockspace_id; }

	void AddMainMenu(const UIMainMenu& menu);
	void AddMenuItem(UIMenuBasePtr menu);
public: // public tool functions
	void ShowConfirmClose();

public: // styles
	UIStyle* CurrentStyle();
	const std::string& CurrentStyleName();
	void SetUIStyle(const std::string& name);

	void UpdateScale(float xscale, float yscale);

	bool IsMainMenuVisible() { return m_show_main_menu; }
	void SetMainMenuVisible(bool visible) { m_show_main_menu = visible; }

	void SetUIMode(E_UI_MODE mode);
	void SetUIRefreshTimeout(double timeout);
	E_UI_MODE GetUIMode() { return m_ui_mode; }
	double GetUIRefreshTimeout() { return m_ui_refresh_timeout; }
public:
	void* GetOldWndProc() { return m_old_wnd_proc; }
	float GetTitleBarHeight() { return m_title_bar_height; }
	void SetMouseCursor(ImGuiMouseCursor cursor) { m_cur_mouse_cursor = cursor; }
protected: // init functions
	void InitNativeWindow(const char* title, int flags);
	void InitImGui();
	void InitStyles();
protected: // functions in the loop
	// these three functions should be called in sequence in the main loop
	void FrameStart();
	void FrameEnd();
	virtual void FrameUpdate() override;

	void CreateRootWindow();
	void MakeAppMenu();
	
	void BeginNativeWindowFrame();
	void EndNativeWindowFrame();
	void DrawHeaderIcon();
	void DrawTitleBar();
protected: // window events
	//! Window close
	virtual void OnWindowClose() override;
	//! Window resize event.
	virtual void OnWindowResize(int theWidth, int theHeight) override;
	//! Key callback
	virtual void OnKey(int key, int scancode, int action, int mods) override;
	//! Content scale callback
	virtual void OnContentScale(float xscale, float yscale) override;
private:
	UIWindowManagerBasePtr m_win_mng;
	// 是否显示菜单
	bool m_show_main_menu = true;
	// UI 刷新模式
	E_UI_MODE m_ui_mode = E_UI_WAIT_TIMEOUT_MODE;
	double m_ui_refresh_timeout = 0.1;
	// 是否要关闭标识
	bool m_close = false;

	// 是否初始化完成
	bool m_is_initialized = false;
	// 菜单列表
	std::vector<UIMainMenu> m_main_menus;
	std::vector<UIMenuBasePtr> m_child_menus;
	
	// 主题列表
	std::map<std::string, UIStyle > m_map_style;
	std::string m_cur_style_name = "blue";

	// dock space id
	ImGuiID m_dockspace_id = 0;

	void* m_old_wnd_proc = NULL;
	float m_title_bar_height = 0.0f;
	ImGuiMouseCursor m_cur_mouse_cursor = ImGuiMouseCursor_Arrow;
};

