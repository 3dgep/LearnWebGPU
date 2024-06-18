#pragma once

#include <chrono>

class Timer
{
public:
    Timer() noexcept;

    // Tick the Timer once per frame.
    void tick() noexcept;

    // Reset the Timer.
    void reset() noexcept;

    // Return the elapsed time in seconds between ticks.
    double elapsedSeconds() const noexcept;

    // Return the elapsed time in milliseconds between ticks.
    double elapsedMilliseconds() const noexcept;

    // Return the elapsed time in microseconds between ticks.
    double elapsedMicroseconds() const noexcept;

    // Return the total time in seconds since the Timer was reset.
    double totalSeconds() const noexcept;

    // Return the total time in milliseconds since the Timer was reset.
    double totalMilliseconds() const noexcept;

    // Return the total time in microseconds since the Timer was reset.
    double totalMicroseconds() const noexcept;

    // Limit the frame-rate to a maximum of fps.
    // Call this every frame you want to simulate a fixed frame rate.
    void limitFPS(int fps) const noexcept;

    // Limit the frame-rate based on the time between frames (in seconds).
    void limitFPS(double seconds) const noexcept;

    // The the frame-rate based on a specific duration.
    void limitFPS(const std::chrono::high_resolution_clock::duration& duration) const noexcept;

    template<class Rep, class Period>
    constexpr void limitFPS(const std::chrono::duration<Rep, Period>& duration) const noexcept
    {
        limitFPS(std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(duration));
    }

private:
    std::chrono::high_resolution_clock::time_point t0, t1;
    mutable std::chrono::high_resolution_clock::time_point beginFrame;

    double elapsedTime = 0.0;
    double totalTime = 0.0;
};