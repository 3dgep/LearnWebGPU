#include <Timer.hpp>
#include <thread>

using namespace std::chrono;

Timer::Timer() noexcept
{
    reset();
}

void Timer::reset() noexcept
{
    t0 = high_resolution_clock::now();
    t1 = t0;
    beginFrame = t0;

    elapsedTime = 0.0;
    totalTime = 0.0;
}

void Timer::tick() noexcept
{
    t1 = high_resolution_clock::now();
    const duration<double, std::micro> delta = t1 - t0;
    t0 = t1;

    elapsedTime = delta.count();
    totalTime += elapsedTime;
}

double Timer::elapsedSeconds() const noexcept
{
    return elapsedTime * 1e-6;
}

double Timer::elapsedMilliseconds() const noexcept
{
    return elapsedTime * 1e-3;
}

double Timer::elapsedMicroseconds() const noexcept
{
    return elapsedTime;
}

double Timer::totalSeconds() const noexcept
{
    return totalTime * 1e-6;
}

double Timer::totalMilliseconds() const noexcept
{
    return totalTime * 1e-3;
}

double Timer::totalMicroseconds() const noexcept
{
    return totalTime;
}

void Timer::limitFPS(int fps) const noexcept
{
    limitFPS(1.0 / static_cast<double>(std::max(1, fps)));
}

void Timer::limitFPS(double seconds) const noexcept
{
    limitFPS(duration<double>(std::max(0.0, seconds)));
}

void Timer::limitFPS(const std::chrono::high_resolution_clock::duration& duration) const noexcept
{
    const auto endTime = beginFrame + duration;
#if 1
    // This uses less CPU power than a busy loop, but it's less accurate.
    std::this_thread::sleep_until(endTime);
#else
    // This busy loop uses more CPU power than this_thread::sleep_until, but it's more accurate.
    while (high_resolution_clock::now() < endTime)
        std::this_thread::yield();
#endif

    beginFrame = endTime;
}