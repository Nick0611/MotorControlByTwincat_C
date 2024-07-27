#pragma once
#include "UIBaseWindow.h"
#include "GLSLShaderToy.h"

class UIMainWindow;

class UIHelpAbout :
    public UIBaseWindow
{
public:
	UIHelpAbout(UIGLWindow* main_win, const char* title);
	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_HELP; }
	virtual const char* GetShowShortCut() { return "F1"; }
protected:
	GLSLShaderToy m_shader_toy;
	bool m_is_in_main_view = true;
	double m_start_time = 0.0;

	UIMainWindow* m_main_win = nullptr;
};

