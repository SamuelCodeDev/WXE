#include "Game.h"
#include "Engine.h"
#include <chrono>
#include <thread>
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

namespace WXE
{
    Graphics*& Game::graphics = Engine::graphics;
    Window*& Game::window = Engine::window;
    Input*& Game::input = Engine::input;
    double& Game::frameTime = Engine::frameTime;

    Game::Game() noexcept 
    {
    }

    Game::~Game() 
    {
    }

    void Game::Draw() 
    {
    }

    void Game::Display() 
    {
    }
    
    void Game::OnPause() 
    { sleep_for(milliseconds(10)); }
}