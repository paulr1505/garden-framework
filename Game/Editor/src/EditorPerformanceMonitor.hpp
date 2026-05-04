#pragma once

#include "Graphics/RenderAPI.hpp"
#include <SDL3/SDL.h>
#include <array>
#include <cstddef>

enum class EditorPerfSeries : size_t
{
    CpuFrame = 0,
    CpuSimulation,
    CpuViewport,
    CpuPIEViewports,
    CpuPrefabPreviews,
    CpuUIBuild,
    CpuPluginTick,
    CpuRenderUI,
    GpuFrame,
    Count
};

class EditorPerformanceMonitor
{
public:
    static constexpr size_t kSeriesCount = static_cast<size_t>(EditorPerfSeries::Count);
    static constexpr int kHistoryCapacity = 360;
    static constexpr float kInvalidSample = -1.0f;

    class ScopedTimer
    {
    public:
        ScopedTimer(EditorPerformanceMonitor& monitor, EditorPerfSeries series)
            : m_monitor(&monitor), m_series(series), m_start(SDL_GetTicksNS())
        {
        }

        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;

        ~ScopedTimer()
        {
            if (m_monitor)
                m_monitor->addSample(m_series, nsToMs(SDL_GetTicksNS() - m_start));
        }

    private:
        EditorPerformanceMonitor* m_monitor = nullptr;
        EditorPerfSeries m_series = EditorPerfSeries::CpuFrame;
        Uint64 m_start = 0;
    };

    void beginFrame()
    {
        m_current.fill(kInvalidSample);
        m_frameActive = true;
    }

    void addSample(EditorPerfSeries series, float milliseconds)
    {
        if (!m_frameActive || milliseconds < 0.0f)
            return;

        const size_t index = static_cast<size_t>(series);
        if (index >= kSeriesCount)
            return;

        if (m_current[index] < 0.0f)
            m_current[index] = 0.0f;
        m_current[index] += milliseconds;
    }

    void endFrame(const RenderFrameStats& renderStats)
    {
        if (!m_frameActive)
            return;

        m_backendName = renderStats.backend_name ? renderStats.backend_name : "Unknown";
        m_completedGpuFrame = renderStats.completed_gpu_frame;
        m_current[static_cast<size_t>(EditorPerfSeries::GpuFrame)] =
            renderStats.gpu_frame_ms_valid ? renderStats.gpu_frame_ms : kInvalidSample;

        for (size_t i = 0; i < kSeriesCount; ++i)
            m_history[i][m_head] = m_current[i];

        m_head = (m_head + 1) % kHistoryCapacity;
        if (m_count < kHistoryCapacity)
            ++m_count;
        m_frameActive = false;
    }

    int historySize() const { return m_count; }
    const char* backendName() const { return m_backendName; }
    uint64_t completedGpuFrame() const { return m_completedGpuFrame; }

    float sample(EditorPerfSeries series, int oldestIndex) const
    {
        if (oldestIndex < 0 || oldestIndex >= m_count)
            return kInvalidSample;

        const size_t series_index = static_cast<size_t>(series);
        if (series_index >= kSeriesCount)
            return kInvalidSample;

        const int start = (m_count == kHistoryCapacity) ? m_head : 0;
        const int index = (start + oldestIndex) % kHistoryCapacity;
        return m_history[series_index][index];
    }

    float latest(EditorPerfSeries series) const
    {
        if (m_count == 0)
            return kInvalidSample;

        const size_t series_index = static_cast<size_t>(series);
        if (series_index >= kSeriesCount)
            return kInvalidSample;

        const int index = (m_head + kHistoryCapacity - 1) % kHistoryCapacity;
        return m_history[series_index][index];
    }

    static float nsToMs(Uint64 nanoseconds)
    {
        return static_cast<float>(static_cast<double>(nanoseconds) / 1000000.0);
    }

private:
    std::array<float, kSeriesCount> m_current{};
    std::array<std::array<float, kHistoryCapacity>, kSeriesCount> m_history{};
    int m_head = 0;
    int m_count = 0;
    bool m_frameActive = false;
    const char* m_backendName = "Unknown";
    uint64_t m_completedGpuFrame = 0;
};
