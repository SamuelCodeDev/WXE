#ifndef ENGINE_H
#define ENGINE_H

#include "Graphics.h"
#include "Window.h"
#include "Input.h"
#include "Timer.h"
#include "Game.h"

namespace WXE
{
    class EngineDesc
    {
    protected:
        static Timer timer;
        static bool paused;

    public:
        static Graphics* graphics;
        static Window* window;
        static Input* input;
        static Game* game;
        static double frameTime;

        static void Pause() noexcept;
        static void Resume() noexcept;
    };

    inline void EngineDesc::Pause() noexcept
    { paused = true; timer.Stop(); }

    inline void EngineDesc::Resume() noexcept
    { paused = false; timer.Start(); }

	class Engine final : public EngineDesc
	{
	private:
		double FrameTime();
		int32 Loop() noexcept;

	public:
        Engine() noexcept;
        ~Engine() noexcept;

        int32 Start(Game* game);

        static LRESULT CALLBACK EngineProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}

#endif