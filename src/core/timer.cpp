#include "timer.hpp"
//! Written entirely by ChatGPT 5.5

HighResolutionTimer::HighResolutionTimer()
{
    Reset();
}

// Call once per frame
void HighResolutionTimer::Tick()
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
void HighResolutionTimer::Reset()
{
    m_previousTime = clock::now();

    m_deltaTime = 0.0;
    m_totalTime = 0.0;

    m_frameCount = 0;
}

// Optional: limit max delta spike
void HighResolutionTimer::SetMaxDeltaTime(double maxDt)
{
    m_maxDeltaTime = maxDt;
}
