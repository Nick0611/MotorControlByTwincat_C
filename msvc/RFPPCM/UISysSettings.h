#pragma once
#include "UIBaseWindow.h"

class UIMainWindow;

class UISysSettings :
    public UIBaseWindow
{
public:
	UISysSettings(UIGLWindow* main_win, const char* title);
	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_SYS; }
	virtual const char* GetShowShortCut() { return "Ctrl+Shift+S"; }
protected:
	UIMainWindow* m_main_win = nullptr;
};

