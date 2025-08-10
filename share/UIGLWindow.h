#pragma once
struct GLFWwindow;
struct GLFWmonitor;
struct GLFWvidmode;

enum UIGLWindowFlags
{
	UIGL_WIN_FLAG_NONE = 0,
	UIGL_WIN_FLAG_MAXMIZE = 1,
	UIGL_WIN_FLAG_NORMAL_CENTER = 1 << 1,
	UIGL_WIN_FLAG_NO_DECORATION = 1 << 2,
	UIGL_WIN_FLAG_NO_TASKBARICON = 1 << 3,
	UIGL_WIN_FLAG_FLOATING = 1 << 4
};

enum class UIGLWindowState
{
	Normal = 0,
	Maximized = 1,
	Iconified = 2,
	Fullscreen = 3
};

class UIGLWindow
{
public:
	UIGLWindow();
	virtual ~UIGLWindow();

	bool CreateGLWindow(int width, int height, const char * title, int flags = UIGL_WIN_FLAG_NONE, GLFWwindow * parent = nullptr);
	void DestroyGLWindow();
	void Show() const;
	void Hide() const;
	bool IsVisible() const;

	GLFWwindow* GetGLWindow() { return m_glfw_window; }
	void* GetNativeHandle() const;
	void* GetNativeGlContext() const;
	const char* GetWindowTitle()  const { return m_window_title; }
	float GetPrimaryMonitorScale()  const { return m_monitor_xscale; }
	bool IsMaximized() const;
	bool IsFullscreen() const;
	bool IsBoarderLess()  const { return m_window_flags & UIGL_WIN_FLAG_NO_DECORATION; }

	void Iconify();
	void Maximize();
	void Restore();
	void Fullscreen();
	void ToggleFullscreen();

	UIGLWindowState GetWindowState() { return m_window_state; }

	void SetPos(int x, int y);
	void SetSize(int w, int h);

	int PosX() { return m_cur_pos[0]; }
	int PosY() { return m_cur_pos[1]; }
	int Width() { return m_cur_size[0]; }
	int Height() { return m_cur_size[1]; }

	void UpdateSize();
	void RefreshWindow();

	void SwapBuffers();

	bool SetWindowIconFromFile(const char* iconfile);
	bool SetWindowIconFromMemoryFile(const unsigned char* icondata, int len);

	void SetWindowMinSize(int minw, int minh);
	int GetWindowMinWidth() { return m_min_limit_window_size[0]; }
	int GetWindowMinHeight() { return m_min_limit_window_size[1]; }

	void SignalClose(int value = 1);

	static UIGLWindow* GetCurrentWindowPtr();
public: // interface
	//! FrameUpdate
	virtual void FrameUpdate() {}
	//! Window close
	virtual void OnWindowClose() {}
	//! Window resize event.
	virtual void OnWindowResize(int theWidth, int theHeight);
	//! Window pos event
	virtual void OnWindowPos(int xpos, int ypos);
	//! Mouse scroll event.
	virtual void OnMouseScroll(double theOffsetX, double theOffsetY) {}
	//! Mouse click event.
	virtual void OnMouseButton(int theButton, int theAction, int theMods) {}
	//! Mouse move event.
	virtual void OnMouseMove(int thePosX, int thePosY) {}
	//! Mouse enter/leave event
	virtual void OnMouseEnter(int entered) {}
	//! Key callback
	virtual void OnKey(int key, int scancode, int action, int mods) {}
	//! Content scale callback
	virtual void OnContentScale(float xscale, float yscale);
protected:
	//！Error callback
	static void OnError(int theError, const char* theDescription);
protected:
	// returns the monitor that contains the greater window area, or NULL if no monitor found
	GLFWmonitor* GetCurrentMonitor();
protected:
	GLFWwindow* m_glfw_parent_window = nullptr;
	GLFWwindow* m_glfw_window = nullptr;
	GLFWmonitor* m_glfw_primary_monitor = nullptr;
	const GLFWvidmode* m_glfw_primary_video_mode = nullptr;
	char m_window_title[256] = { 0 };
	
	float m_monitor_xscale = 1.0;
	float m_monitor_yscale = 1.0;

	int m_cur_pos[2] = { 0 };
	int m_cur_size[2] = { 0 };

	int m_last_pos[2] = { 0, 0 };
	int m_last_size[2] = { 800, 600 };
	// 主屏幕窗口最大值
	int m_maximum_size[2] = { 0,0 };
	// 窗口最小值，不允许比这个值更小,borderless 模式下有效
	int m_min_limit_window_size[2] = { 640, 480 };

	UIGLWindowState m_window_state = UIGLWindowState::Normal;
	int m_window_flags = UIGL_WIN_FLAG_NONE;
};

