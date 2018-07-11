#include "window.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool Window::Startup(uint32_t Width, uint32_t Height, const std::string& Title)
{
	mWidth = Width;
	mHeight = Height;
	mTitle = Title;

	// Create window class
	WNDCLASSEX WinClass = {};
	WinClass.cbSize = sizeof(WNDCLASSEX);
	WinClass.hInstance = GetModuleHandle(nullptr);
	WinClass.lpszClassName = "VulkantasticWinClass";
	WinClass.lpfnWndProc = &WindowProc;

	if (!RegisterClassEx(&WinClass))
	{
		return false;
	}

	// Create window
	mHandle = CreateWindowEx(0, WinClass.lpszClassName, mTitle.c_str(), WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, mWidth, mHeight, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
	if (!mHandle)
	{
		return false;
	}

	ShowWindow(mHandle, SW_SHOWNORMAL);
	UpdateWindow(mHandle);

	mIsRunning = true;

	return true;
}


bool Window::Shutdown()
{
	PostQuitMessage(0);
	DestroyWindow(mHandle);

	return true;
}

void Window::Update()
{
	MSG Message;
	while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
	{
		if (Message.message == WM_QUIT)
		{
			Close();
		}

		TranslateMessage(&Message);
		DispatchMessageA(&Message);
	}
}

void Window::Close()
{
	mIsRunning = false;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (uMsg)
	{

	case WM_CLOSE:
	{
		Window::Get().Close();
		break;
	}
	case WM_DESTROY:
	{
		break;
	}
	default:
	{
		result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	}

	return result;
}

