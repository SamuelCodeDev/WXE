#include "Window.h"

#ifdef _WIN32
void GetSizeScreen(WXE::uint32 width, WXE::uint32 height) {
    width = GetSystemMetrics(SM_CXSCREEN);
    height = GetSystemMetrics(SM_CYSCREEN);
}
#elif __linux
void GetSizeScreen(WXE::uint32 width, WXE::uint32 height) {
    Display * display = XOpenDisplay(nullptr);
    Screen * scrn = DefaultScreenOfDisplay(display);
    
	height = scrn->height;
    width  = scrn->width;
    
	delete scrn;
    XCloseDisplay(display);
}
#endif

#ifdef _WIN32

namespace WXE::Windows
{
	void (*Window::inFocus)() = nullptr;
	void (*Window::lostFocus)() = nullptr;

	Window::Window() noexcept
	{
		hInstance = GetModuleHandle(nullptr);
		windowHandle = 0;
		GetSizeScreen(windowWidth, windowHeight);
		windowIcon = LoadIcon(nullptr, IDI_APPLICATION);
		windowCursor = LoadCursor(nullptr, IDC_ARROW);
		windowColor = RGB(255, 255, 255);
		windowTitle = string("Windows Game");
		windowStyle = WS_POPUP | WS_VISIBLE;
		windowMode = FULLSCREEN;
		windowPosX = 0;
		windowPosY = 0;
		windowCenterX = windowWidth / 2.0f;
		windowCenterY = windowHeight / 2.0f;
	}

	void Window::Mode(const int32 mode) noexcept
	{
		windowMode = mode;

		if (mode == WINDOWED) windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE;
		else				  windowStyle = WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE;
	}

	void Window::Size(const int32 width, const int32 height) noexcept
	{
		windowWidth = width;
		windowHeight = height;

		windowCenterX = windowWidth / 2.0f;
		windowCenterY = windowHeight / 2.0f;

		windowPosX = GetSystemMetrics(SM_CXSCREEN) / 2.0f - windowWidth / 2.0f;
		windowPosY = GetSystemMetrics(SM_CYSCREEN) / 2.0f - windowHeight / 2.0f;
	}

    bool Window::Create()
    {
		WNDCLASSEX wndClass {
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = Window::WinProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hInstance,
			.hIcon = windowIcon,
			.hCursor = windowCursor,
			.hbrBackground = static_cast<HBRUSH>(CreateSolidBrush(windowColor)),
			.lpszMenuName = nullptr,
			.lpszClassName = "GameWindow",
			.hIconSm = windowIcon,
		};

        if (!RegisterClassEx(&wndClass))
            return false;

        windowHandle = CreateWindowEx(
            0,
            "GameWindow",
            windowTitle.c_str(),
            windowStyle,
			static_cast<int32>(windowPosX), static_cast<int32>(windowPosY),
            windowWidth, windowHeight,
			nullptr,
			nullptr,
            hInstance,
            nullptr
		);

        if (windowMode == WINDOWED)
        {
            RECT rect { 0, 0, windowWidth, windowHeight };

            AdjustWindowRectEx(&rect,
                GetWindowStyle(windowHandle),
                GetMenu(windowHandle) != nullptr,
                GetWindowExStyle(windowHandle));

			windowPosX = (GetSystemMetrics(SM_CXSCREEN) / 2.0f) - ((rect.right - rect.left) / 2.0f);
			windowPosY = (GetSystemMetrics(SM_CYSCREEN) / 2.0f) - ((rect.bottom - rect.top) / 2.0f);

            MoveWindow(
                windowHandle,
				static_cast<int32>(windowPosX), static_cast<int32>(windowPosY),
                rect.right - rect.left,
                rect.bottom - rect.top,
                TRUE
			);
        }

        return (windowHandle ? true : false);
    }

	LRESULT CALLBACK Window::WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) 
		{
		case WM_KILLFOCUS:
			if (lostFocus)
				lostFocus();
			return 0;

		case WM_SETFOCUS:
			if (inFocus)
				inFocus();
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

#elif __linux__

#include <cstdlib>
#include "Utils.h"

namespace WXE::Linux
{
	Window::Window() noexcept : screenNum{}
	{
		windowPosX = 0;
		windowPosY = 0;
		display = XOpenDisplay(nullptr);
		GetSizeScreen(windowWidth, windowHeight);
	}

	Window::~Window()
	{
		XFlush(display);
		XCloseDisplay(display);
	}

	bool Window::Create()
	{
		display = XOpenDisplay(getenv("DISPLAY"));
		if (display == nullptr)
			return false;

		screenNum = DefaultScreen(display);

		window = XCreateSimpleWindow(
			display, 
			RootWindow(display, screenNum),
			windowPosX, windowPosY, 
			windowWidth, windowHeight,
			2,
			BlackPixel(display, screenNum),
			WhitePixel(display, screenNum)
		);

		XMapWindow(display, window);
		XFlush(display);

		screenNum = DefaultScreen(display);

		{
			XGCValues values;
			gc = XCreateGC(display, window, 0, &values);
			if (gc < 0)
				return false;
		}

		XSetForeground(display, gc, BlackPixel(display, screenNum));
		XSetBackground(display, gc, WhitePixel(display, screenNum));
		XSetLineAttributes(display, gc, 2, LineSolid, CapButt, JoinBevel);
		XSetFillStyle(display, gc, FillSolid);
		XFlush(display);
	}

    void Window::Size(const uint32 width, const uint32 height) noexcept
    {
		windowWidth = width;
		windowHeight = height;

		windowCenterX = windowWidth / 2.0f;
		windowCenterY = windowHeight / 2.0f;

		{
			uint32 widthScreen, heightScreen;
			GetSizeScreen(widthScreen, heightScreen); 
			windowPosX = widthScreen / 2.0f - windowWidth / 2.0f;
			windowPosY = heightScreen / 2.0f - windowHeight / 2.0f;
		}
		//XResizeWindow(display, window, width, height);
      	//XFlush(display);
    }
}

#endif