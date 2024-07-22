#ifndef GAME_H
#define GAME_H

#include "Window.h"
#include "Input.h"
#include "Graphics.h"

#ifdef _WIN32
    using Window = WXE::Windows::Window;
    using Input = WXE::Inputs::Input;
    using Graphics = WXE::DX12::Graphics;
#elif __linux__
    using Window = WXE::Linux::Window;
    using Input = WXE::Inputs::Input;
    //using Graphics = WXE::DX12::Graphics;
#endif

namespace WXE
{
	class Game
	{
    protected:
        static Graphics*& graphics;
        static Window*& window;
        static Input*& input;
        static double& frameTime;

    public:
        Game() noexcept;
        virtual ~Game();

        virtual void Init() = 0;
        virtual void Update() = 0;
        virtual void Finalize() = 0;

        virtual void Draw();
        virtual void Display();
        virtual void OnPause();
	};
}

#endif