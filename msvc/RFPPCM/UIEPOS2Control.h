#pragma once
#include "UIBaseWindow.h"
#include <vector>
#include <string>
#include <memory>
#include "EPOS2.h"

namespace Device {
	class EPOS2;
};
//using namespace Device;
class UIEPOS2Control : public UIBaseWindow
{
public:
	UIEPOS2Control(UIGLWindow* main_win, const char* title);

	void AttachEPOS2Ptr(size_t index, std::shared_ptr<Device::EPOS2> epos2);
	std::shared_ptr<Device::EPOS2> GetEPOS2Ptr(size_t index);

	virtual void Draw();

	virtual EUIMenuCategory GetWinMenuCategory() { return EUIMenuCategory::E_UI_CAT_TOOL; }
	virtual const char* GetShowShortCut() { return "Ctrl+3"; }
protected:
	std::vector<std::shared_ptr<Device::EPOS2>> m_vec_epos2;

	int m_selected_index = -1;

	char m_epos_sdk_name[64] = { 0 };
	char m_epos_sdk_ver[64] = { 0 };
};

