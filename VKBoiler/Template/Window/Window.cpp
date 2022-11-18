#include "Window.h"

#include <stdio.h>
#include <string>

const char* Window::s_WindowClassName = "VkBoilerWC";

// Returns Window*
#define GET_WINDOW_HANDLE(hWnd) reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, 0))

Window::Window(const std::string& appName, bool allowResizing, uint32_t width, uint32_t height)
{
	m_Width = width;
	m_Height = height;
	RegisterWC();
	CreateHWND(appName, allowResizing);
}

Window::~Window()
{
	DestroyWindow(m_Handle);
}

void Window::Tick()
{
	MSG msg{};
	while (PeekMessageA(&msg, m_Handle, 0, 0, PM_REMOVE) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void Window::CreateHWND(const std::string& appName, bool allowResizing)
{
	std::string title = "VkBoiler: ";
	title.append(appName);

	DWORD style = WS_OVERLAPPEDWINDOW;
	if (!allowResizing)
		style &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);

	// Adjust the window rectangle to ensure the 'drawing' surface matches the given width and height members
	RECT rect{};
	rect.left = 0;
	rect.top = 0;
	rect.right = static_cast<LONG>(m_Width);
	rect.bottom = static_cast<LONG>(m_Height);

	::AdjustWindowRect(&rect, style, FALSE);

	rect.right -= rect.left;
	rect.bottom -= rect.top;


	m_Handle = CreateWindowA(
		s_WindowClassName,
		title.c_str(),
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		static_cast<int>(rect.right), static_cast<int>(rect.bottom),
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL
	);

	if (m_Handle == NULL)
		return;

	SetWindowLongPtrA(m_Handle, 0, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(m_Handle, SW_SHOW);
}

/*static*/void Window::RegisterWC()
{
	WNDCLASSA wc {};
	HMODULE application_module = GetModuleHandle(NULL);

	if (!GetClassInfoA(application_module, s_WindowClassName, &wc))
	{
		wc = {};
		wc.style = NULL;
		wc.lpfnWndProc = Window::WndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(Window*);
		wc.hInstance = application_module;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(wc.hInstance, IDC_ARROW);
		wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
		wc.lpszMenuName = NULL;
		wc.lpszClassName = s_WindowClassName;

		RegisterClassA(&wc);
	}
}


LRESULT Window::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
		case WM_CLOSE:
		{
			Window* window = GET_WINDOW_HANDLE(hWnd);
			if (window != nullptr)
				window->m_QuitMessage = true;

			PostQuitMessage(0);
			return 0;
		}

		default:
		{
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
	}
}