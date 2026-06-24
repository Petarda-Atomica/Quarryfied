#pragma once

//! Written entirely by ChatGPT 5.5

#include <chrono>
#include <algorithm>

class HighResolutionTimer
{
public:
    using clock = std::chrono::steady_clock;

    HighResolutionTimer();

    // Call once per frame
    void Tick();

    // Reset timer state
    void Reset();

    // Delta time in seconds
    double DeltaTime() const { return m_deltaTime; }

    // Delta time in milliseconds
    double DeltaTimeMs() const { return m_deltaTime * 1000.0; }

    // Total running time in seconds
    double TotalTime() const { return m_totalTime; }

    // Frames processed
    uint64_t FrameCount() const { return m_frameCount; }

    // FPS estimate
    double FPS() const
    {
        return (m_deltaTime > 0.0)
            ? (1.0 / m_deltaTime)
            : 0.0;
    }

    // Optional: limit max delta spike
    void SetMaxDeltaTime(double maxDt);

private:
    clock::time_point m_previousTime;

    double m_deltaTime = 0.0;
    double m_totalTime = 0.0;

    double m_maxDeltaTime = 0.25; // 250 ms cap

    uint64_t m_frameCount = 0;
};
