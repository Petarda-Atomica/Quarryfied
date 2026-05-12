#pragma once

//! Written entirely by ChatGPT 5.5

#include <chrono>
#include <algorithm>

class HighResolutionTimer
{
public:
    using clock = std::chrono::steady_clock;

    HighResolutionTimer()
    {
        Reset();
    }

    // Call once per frame
    void Tick()
    {
        const auto currentTime = clock::now();

        m_deltaTime =
            std::chrono::duration<double>(currentTime - m_previousTime).count();

        m_previousTime = currentTime;

        // Prevent huge spikes after breakpoints, window dragging, etc.
        m_deltaTime = std::clamp(m_deltaTime, 0.0, m_maxDeltaTime);

        m_totalTime += m_deltaTime;

        ++m_frameCount;
    }

    // Reset timer state
    void Reset()
    {
        m_previousTime = clock::now();

        m_deltaTime = 0.0;
        m_totalTime = 0.0;

        m_frameCount = 0;
    }

    // Delta time in seconds
    double DeltaTime() const
    {
        return m_deltaTime;
    }

    // Delta time in milliseconds
    double DeltaTimeMs() const
    {
        return m_deltaTime * 1000.0;
    }

    // Total running time in seconds
    double TotalTime() const
    {
        return m_totalTime;
    }

    // Frames processed
    uint64_t FrameCount() const
    {
        return m_frameCount;
    }

    // FPS estimate
    double FPS() const
    {
        return (m_deltaTime > 0.0)
            ? (1.0 / m_deltaTime)
            : 0.0;
    }

    // Optional: limit max delta spike
    void SetMaxDeltaTime(double maxDt)
    {
        m_maxDeltaTime = maxDt;
    }

private:
    clock::time_point m_previousTime;

    double m_deltaTime = 0.0;
    double m_totalTime = 0.0;

    double m_maxDeltaTime = 0.25; // 250 ms cap

    uint64_t m_frameCount = 0;
};
