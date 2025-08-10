#pragma once

enum class E_UI_LOG_LEVEL{
	E_UI_LOG_DEBUG,
	E_UI_LOG_INFO,
	E_UI_LOG_WARN,
	E_UI_LOG_ERROR,
	E_UI_LOG_FATAL
};


class UILog
{
public:
	static UILog* Inst();

	void    Clear();

	void    AddLog(E_UI_LOG_LEVEL level, const char* fmt, ...);

	void    Draw(const char* title, bool* p_open = nullptr);
protected:
	UILog();
	~UILog();
protected:
	struct UILogCtx;
	UILogCtx * ctx;
};

// useful macros
#define UI_DEBUG(fmt,...)				UILog::Inst()->AddLog(E_UI_LOG_LEVEL::E_UI_LOG_DEBUG, fmt, __VA_ARGS__)
#define UI_INFO(fmt,...)				UILog::Inst()->AddLog(E_UI_LOG_LEVEL::E_UI_LOG_INFO,  fmt, __VA_ARGS__)
#define UI_WARN(fmt,...)				UILog::Inst()->AddLog(E_UI_LOG_LEVEL::E_UI_LOG_WARN,  fmt, __VA_ARGS__)
#define UI_ERROR(fmt,...)				UILog::Inst()->AddLog(E_UI_LOG_LEVEL::E_UI_LOG_ERROR, fmt, __VA_ARGS__)
#define UI_FATAL(fmt,...)				UILog::Inst()->AddLog(E_UI_LOG_LEVEL::E_UI_LOG_FATAL, fmt, __VA_ARGS__)
