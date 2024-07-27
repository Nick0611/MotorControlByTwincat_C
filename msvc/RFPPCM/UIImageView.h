#pragma once
#include "UIBaseWindow.h"


class UIImageBase;

class UIImageView :
    public UIBaseWindow
{
public:
	UIImageView(UIGLWindow* main_win, const char* title);
	virtual ~UIImageView();

	virtual void Draw();

	void ShowImageFromFile(const char* filename);

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_POPUP; }
protected:
	UIImageBase* m_image = nullptr;
	float m_image_zoom = 1.0f;
};

