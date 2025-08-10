#pragma once
#include <future>
#include <vector>
#include "imgui/imgui.h"

class UIFontLoader
{
public:
	static UIFontLoader* Inst()
	{
		static UIFontLoader loader;
		return &loader;
	}
	void SetExtraChars(const ImWchar* data, size_t size);
	void LoadFont();

	ImFontAtlas* GetFontAtlas();
	ImFont* GetFontH1() { return m_font_h1; }
	ImFont* GetFontH2() { return m_font_h2; }
	ImFont* GetFontH3() { return m_font_h3; }
protected:
	UIFontLoader();
	~UIFontLoader();

	void _DoLoad();
protected:
	std::vector<ImWchar> m_extra_chars;
	std::future<void> m_load_font_finish;
	ImFontAtlas* m_font_atlas = nullptr;

	ImFont* m_font_h1 = nullptr;
	ImFont* m_font_h2 = nullptr;
	ImFont* m_font_h3 = nullptr;
};
