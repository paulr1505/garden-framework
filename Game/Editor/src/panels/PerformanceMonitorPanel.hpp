#pragma once

#include "EditorPerformanceMonitor.hpp"
#include <array>

class PerformanceMonitorPanel
{
public:
    PerformanceMonitorPanel();

    void draw(const EditorPerformanceMonitor& monitor, bool* p_open = nullptr);

private:
    std::array<bool, EditorPerformanceMonitor::kSeriesCount> m_seriesVisible{};
};
