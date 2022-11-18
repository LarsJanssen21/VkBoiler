#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdint.h>
#include <string>

class Window
{
public:
	Window(const std::string& appName, bool allowResizing, uint32_t width, uint32_t height);
	~Window();

	void Tick();

	uint32_t GetWidth() const { return m_Width; }
	uint32_t GetHeight() const { return m_Height; }

	HWND GetWindowHandle() const { return m_Handle; }

	bool IsValid() const { return m_Handle != NULL; }
	bool WantsQuit() const { return m_QuitMessage; }

private:
	void CreateHWND(const std::string& appName, bool allowResizing);
	static void RegisterWC();

	static LRESULT WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
private:
	HWND m_Handle = NULL;
	uint32_t m_Width = 640, m_Height = 640;
	static const char* s_WindowClassName;
	bool m_QuitMessage = false;
};