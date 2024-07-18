#ifndef INPUT_H
#define INPUT_H

#include "Window.h"

namespace WXE::Inputs
{
	enum { MAX_KEYS = 256 };

	class Keyboard
	{
	protected:
		static bool keys[MAX_KEYS];
		static bool ctrl[MAX_KEYS];
		static string text;

	public:
		bool KeyDown(const uint8 vkcode) const noexcept;
		bool KeyUp(const uint8 vkcode) const noexcept;
		bool KeyPress(const uint8 vkcode) const noexcept;
		
		static const char* Text() noexcept;
	};

	inline bool Keyboard::KeyDown(const uint8 vkcode) const noexcept
	{ return keys[vkcode]; }

	inline bool Keyboard::KeyUp(const uint8 vkcode) const noexcept
	{ return !(keys[vkcode]); }

	inline const char* Keyboard::Text() noexcept
	{ return text.c_str(); }

	class Mouse
	{
	protected:
		static int16 mouseWheel;
		static int32 mouseX;
		static int32 mouseY;

	public:
		int32 MouseX() const noexcept;
		int32 MouseY() const noexcept;
		int16 MouseWheel() noexcept;
	};

	inline int32 Mouse::MouseX() const noexcept
	{ return mouseX; }

	inline int32 Mouse::MouseY() const noexcept
	{ return mouseY; }

	inline int16 Mouse::MouseWheel() noexcept
	{
		int16 val = mouseWheel;
		mouseWheel = 0;
		return val;
	}

	class Input final : public Keyboard, public Mouse
	{
	public:
		Input() noexcept;
		~Input() noexcept;

		void Read() const noexcept;

		static LRESULT CALLBACK Reader(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK InputProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};

	inline void Input::Read() const noexcept
	{
		text.clear();
		SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Input::Reader));
	}
}

#endif