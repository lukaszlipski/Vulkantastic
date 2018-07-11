#pragma once
#include <string>
#include <windows.h>

class Window
{
public:
	static Window& Get()
	{
		static Window *instance = new Window();
		return *instance;
	}

	bool Startup(uint32_t Width, uint32_t Height, const std::string& Title);
	bool Shutdown();

	void Update();
	void Close();

	inline bool ShouldWindowClose() const { return !mIsRunning; }
	inline std::string GetTitle() const { return mTitle; }
	HWND GetWindowHandleWin32() const { return mHandle; }

private:
	Window() = default;

	std::string mTitle;
	int32_t mWidth;
	int32_t mHeight;
	bool mIsRunning = false;
	HWND mHandle;

};