#pragma once
#include <vector>
#include <string>
#include "UIWindowManagerBase.h"
#include "UILogView.h"
#include "UISysSettings.h"
#include "UIImageView.h"
//#include "UIPPCMVar.h"
//#include "UIPPCMPlot.h"
#include "UIOptiTrack.h"
#include "UISensorMonitor.h"
#include "UIEPOS2Control.h"
#include "UIHypersenMonitor.h"
#include "UIFinRay.h"
#include "UITrolleyControlTest.h"
#include "UIForceSensor_HUATRAN.h"

class UIMainWindow;
class UIApplication;

class UIWindowManager: public UIWindowManagerBase
{
public:
	UIWindowManager(UIMainWindow * ui_main);
	~UIWindowManager();

	void Init() override;

	void Draw() override;

	void OnCPSMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len) override;

	void Destroy() override;
	
	UISysSettings* GetSysSettings() { return m_ui_sys; }
	UIEPOS2Control* GetEPOS2Control() { return m_ui_epos2; }
	UISensorMonitor* GetSensorMonitor() { return m_ui_sensor; }
	UITrolleyControlTest* GetTrolleyControlTest() { return m_ui_trolley; }
	UIForceSensor_HUATRAN* GetForceSensor_HUATRAN() { return m_ui_force_huatran; }
	//UIPPCMPlot* GetPPCMPlot() { return m_ui_plot; }
public:
	void ShowOpenModelFileDialog();
	void ShowSaveModelFileDialog();

	void OpenFile(const std::string& filename);
	void SaveFile(const std::string& filename);

	void SetResetLayoutFlag() { m_reset_layout = true; }

	void ShowAllWindow();
	void HideAllWindow();
	void SetShowDemo(bool show) { m_show_demo = show; }
#ifdef UI_CFG_USE_IMPLOT
	void SetShowPlotDemo(bool show) { m_show_plot_demo = show; }
#endif
protected:
	void InitWindows();
	void InitMenus();
	void InitImDialog();
	void ResetLayout();

	void ShowDemo();
#ifdef UI_CFG_USE_IMPLOT
	void ShowPlotDemo();
#endif
protected:
	// ´°¿ÚÁÐ±í
	std::vector<UIBaseWindow*> m_win_list;

	UILogView* m_ui_log = nullptr;
	UISysSettings* m_ui_sys = nullptr;
	UIImageView* m_ui_image = nullptr;

	//UIPPCMVar* m_ui_var = nullptr;
	//UIPPCMPlot* m_ui_plot = nullptr;
	UIOptiTrack* m_ui_opt = nullptr;
	UISensorMonitor* m_ui_sensor = nullptr;
	UIEPOS2Control* m_ui_epos2 = nullptr;
	UIHypersenMonitor* m_ui_hypersen = nullptr;
	UIFinRay* m_ui_finray = nullptr;
	UITrolleyControlTest* m_ui_trolley = nullptr;
	UIForceSensor_HUATRAN* m_ui_force_huatran = nullptr;

	// window show
	bool m_show_demo = false;
#ifdef UI_CFG_USE_IMPLOT
	bool m_show_plot_demo = false;
#endif

	bool m_reset_layout = true;
};

using UIWindowManagerPtr = std::shared_ptr<UIWindowManager>;
