#ifndef TIMER_H
#define TIMER_H

#include <chrono>

namespace WXE
{
	using Clock = std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;

	class Timer final
	{
	private:
		Clock::time_point start, end;
		bool stoped;

	public:
		Timer() noexcept;

		void Start() noexcept;
		void Stop() noexcept;
		double Reset() noexcept;
		double Elapsed() const noexcept;
		bool Elapsed(const double secs) const noexcept;

		Clock::time_point Stamp() noexcept;
		double Elapsed(Clock::time_point stamp) const noexcept;
		bool Elapsed(Clock::time_point stamp, const double secs) const noexcept;
	};

	inline double Timer::Elapsed(Clock::time_point stamp) const noexcept
	{
		if (stoped) return duration_cast<duration<double>>(end - stamp).count();
		else        return duration_cast<duration<double>>(Clock::now() - stamp).count();
	}

	inline double Timer::Elapsed() const noexcept
	{
		if (stoped) return duration_cast<duration<double>>(end - start).count();
		else        return duration_cast<duration<double>>(Clock::now() - start).count();
	}

	inline Clock::time_point Timer::Stamp() noexcept
	{ return Clock::now(); }

	inline bool Timer::Elapsed(const double secs) const noexcept
	{ return (Elapsed() >= secs ? true : false); }

	inline bool Timer::Elapsed(Clock::time_point stamp, const double secs) const noexcept
	{ return (Elapsed(stamp) >= secs ? true : false); }
}

#endif