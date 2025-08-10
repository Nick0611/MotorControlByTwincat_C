#pragma once
#include <string>
#include <chrono>
#include "imgui/imgui.h"
#include "UIDef.h"


class UIUtils
{
public:
	static UIUtils* Inst()
	{
		static UIUtils util;
		return &util;
	}
	// must be called from the main thread
	bool InitGlad();

	// must be called from the main thread after glfwInit
	bool LoadTextureFromFile(const char* filename, unsigned int* out_texture, int* out_width, int* out_height);

	bool LoadImageFromFile(const char* filename, unsigned char ** out_image_data, int* out_width, int* out_height);
	bool LoadImageFromMemory(const unsigned char* data, int len, unsigned char** out_image_data, int* out_width, int* out_height);
	void FreeImageData(unsigned char*& in_image_data);
	void DeleteTexture(unsigned int* texture_id);
	// must be called from the main thread after glfwInit
	unsigned int CreateTexture(unsigned char* in_image_data, int in_width, int in_height, int channels=4);

	bool LoadTextureFromFile(const char* filename, ST_UITexture & texture);
	void DeleteTexture(ST_UITexture* texture);

	int ShowMessageBox(const char * title, const char * content);

	bool ReadTxtFile(const std::string& filename, std::string& content);
	bool IsFileExists(const std::string& filename);
protected:
	UIUtils();
	~UIUtils();

	bool m_is_glad_init = false;
};

// 简单的Timer类，只可用于MainLoop中
class UITimer
{
public:
	UITimer();
	void StartTimer(double interval_sec);
	void StopTimer();
	bool CheckTimer();
protected:
	// a simple timer
	double m_last_check_time_sec = 0;
	bool m_timer_enable = false;
	double m_timer_interval_sec = 0.0f;
};
