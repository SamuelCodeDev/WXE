#include "Input.h"
#include "KeyCodes.h"

namespace WXE::Inputs
{
	bool Keyboard::keys[MAX_KEYS] = {};
	bool Keyboard::ctrl[MAX_KEYS] = {};
	string Keyboard::text;

	int32 Mouse::mouseX = {};
	int32 Mouse::mouseY = {};
	int16 Mouse::mouseWheel = {};

	Input::Input() noexcept
	{ SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Input::InputProc)); }

	Input::~Input() noexcept
	{ SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Windows::Window::WinProc)); }

	bool Keyboard::KeyPress(const uint8 vkcode) const noexcept
	{
		if (ctrl[vkcode])
		{
			if (KeyDown(vkcode))
			{
				ctrl[vkcode] = false;
				return true;
			}
		}
		else if (KeyUp(vkcode))
		{
			ctrl[vkcode] = true;
		}

		return false;
	}

    LRESULT CALLBACK Input::Reader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_CHAR)
        {
            switch (wParam)
            {
            case VK_BACK:
                if (!text.empty()) text.erase(text.size() - 1);
                break;

            case VK_TAB:
            case VK_RETURN:
                SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Input::InputProc));
                break;

            default:
                text += static_cast<char>(wParam);
                break;
            }

            return 0;
        }

        return CallWindowProc(Input::InputProc, hWnd, msg, wParam, lParam);
    }

    LRESULT CALLBACK Input::InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_KEYDOWN:
            keys[wParam] = true;
            return 0;

        case WM_KEYUP:
            keys[wParam] = false;
            return 0;

        case WM_MOUSEMOVE:
            mouseX = GET_X_LPARAM(lParam);
            mouseY = GET_Y_LPARAM(lParam);
            return 0;

        case WM_MOUSEWHEEL:
            mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
            return 0;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            keys[VK_LBUTTON] = true;
            return 0;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
            keys[VK_MBUTTON] = true;
            return 0;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
            keys[VK_RBUTTON] = true;
            return 0;

        case WM_LBUTTONUP:
            keys[VK_LBUTTON] = false;
            return 0;

        case WM_MBUTTONUP:
            keys[VK_MBUTTON] = false;
            return 0;

        case WM_RBUTTONUP:
            keys[VK_RBUTTON] = false;
            return 0;
        }

        return CallWindowProc(Windows::Window::WinProc, hWnd, msg, wParam, lParam);
    }
}