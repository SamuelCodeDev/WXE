#include "Timer.h"

namespace WXE
{
    Timer::Timer() noexcept : stoped{ false }
    {
    }

    void Timer::Start() noexcept
    {
        if (stoped)
        {
            start = Clock::now() - (end - start);
            stoped = false;
        }
        else
        {
            start = Clock::now();
        }
    }

    void Timer::Stop() noexcept
    {
        if (!stoped)
        {
            end = Clock::now();
            stoped = true;
        }
    }

    double Timer::Reset() noexcept
    {
        double elapsed;

        if (stoped)
        {
            elapsed = duration_cast<duration<double>>(end - start).count();
            start = Clock::now();
            stoped = false;
        }
        else
        {
            end = Clock::now();
            elapsed = duration_cast<duration<double>>(end - start).count();
            start = end;
        }

        return elapsed;
    }
}