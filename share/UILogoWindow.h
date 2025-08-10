#pragma once
#include "UIBaseWindow.h"
class UILogoWindow :
    public UIBaseWindow
{
public:
	UILogoWindow(UIGLWindow* main_win, const char* title);

	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_POPUP; }
};

