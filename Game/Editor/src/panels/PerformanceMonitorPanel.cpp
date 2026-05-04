#include "PerformanceMonitorPanel.hpp"
#include "EditorIcons.hpp"
#include "PanelUtils.hpp"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace
{
    struct SeriesStyle
    {
        EditorPerfSeries series;
        const char* label;
        ImU32 color;
    };

    const std::array<SeriesStyle, EditorPerformanceMonitor::kSeriesCount>& seriesStyles()
    {
        static const std::array<SeriesStyle, EditorPerformanceMonitor::kSeriesCount> styles = {{
            { EditorPerfSeries::CpuFrame,          "CPU Frame",       IM_COL32(242, 205, 80, 255) },
            { EditorPerfSeries::CpuSimulation,     "Simulation",      IM_COL32(112, 190, 255, 255) },
            { EditorPerfSeries::CpuViewport,       "Viewport",        IM_COL32(85, 220, 170, 255) },
            { EditorPerfSeries::CpuPIEViewports,   "PIE Viewports",   IM_COL32(130, 165, 255, 255) },
            { EditorPerfSeries::CpuPrefabPreviews, "Prefab Previews", IM_COL32(255, 155, 95, 255) },
            { EditorPerfSeries::CpuUIBuild,        "UI Build",        IM_COL32(205, 145, 255, 255) },
            { EditorPerfSeries::CpuPluginTick,     "Plugin Tick",     IM_COL32(255, 115, 145, 255) },
            { EditorPerfSeries::CpuRenderUI,       "Render UI",       IM_COL32(150, 230, 95, 255) },
            { EditorPerfSeries::GpuFrame,          "GPU Frame",       IM_COL32(255, 95, 215, 255) },
        }};
        return styles;
    }

    bool validSample(float value)
    {
        return value >= 0.0f && std::isfinite(value);
    }
}

PerformanceMonitorPanel::PerformanceMonitorPanel()
{
    m_seriesVisible.fill(false);
    m_seriesVisible[static_cast<size_t>(EditorPerfSeries::CpuFrame)] = true;
    m_seriesVisible[static_cast<size_t>(EditorPerfSeries::CpuViewport)] = true;
    m_seriesVisible[static_cast<size_t>(EditorPerfSeries::CpuRenderUI)] = true;
    m_seriesVisible[static_cast<size_t>(EditorPerfSeries::GpuFrame)] = true;
}

void PerformanceMonitorPanel::draw(const EditorPerformanceMonitor& monitor, bool* p_open)
{
    if (!ImGui::Begin("Performance Monitor", p_open))
    {
        ImGui::End();
        return;
    }

    PanelMaximizeButton();

    ImGui::Text("%s Performance Monitor", ICON_FA_GAUGE_HIGH);
    ImGui::SameLine();
    ImGui::TextDisabled("Backend: %s", monitor.backendName());
    if (monitor.completedGpuFrame() > 0)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("GPU sample #%llu",
                            static_cast<unsigned long long>(monitor.completedGpuFrame()));
    }

    ImGui::Separator();

    const int history_size = monitor.historySize();
    ImVec2 graph_size(ImGui::GetContentRegionAvail().x, 240.0f);
    graph_size.x = std::max(graph_size.x, 120.0f);

    float max_ms = 0.0f;
    for (const SeriesStyle& style : seriesStyles())
    {
        if (!m_seriesVisible[static_cast<size_t>(style.series)])
            continue;

        for (int i = 0; i < history_size; ++i)
        {
            const float value = monitor.sample(style.series, i);
            if (validSample(value))
                max_ms = std::max(max_ms, value);
        }
    }
    max_ms = std::max(max_ms, 16.67f);
    max_ms = std::ceil(max_ms / 5.0f) * 5.0f;

    ImGui::InvisibleButton("##perf_graph", graph_size);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_FrameBg), 4.0f);
    draw_list->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_Border), 4.0f);

    for (int line = 1; line <= 4; ++line)
    {
        const float t = static_cast<float>(line) / 4.0f;
        const float y = min.y + (max.y - min.y) * t;
        draw_list->AddLine(ImVec2(min.x, y), ImVec2(max.x, y),
                           IM_COL32(255, 255, 255, 28), 1.0f);
    }

    char label[32];
    snprintf(label, sizeof(label), "%.1f ms", max_ms);
    draw_list->AddText(ImVec2(min.x + 8.0f, min.y + 6.0f),
                       ImGui::GetColorU32(ImGuiCol_TextDisabled), label);
    snprintf(label, sizeof(label), "%.1f ms", max_ms * 0.5f);
    draw_list->AddText(ImVec2(min.x + 8.0f, min.y + graph_size.y * 0.5f - ImGui::GetTextLineHeight()),
                       ImGui::GetColorU32(ImGuiCol_TextDisabled), label);

    draw_list->PushClipRect(min, max, true);
    if (history_size > 1)
    {
        const float width = max.x - min.x;
        const float height = max.y - min.y;
        for (const SeriesStyle& style : seriesStyles())
        {
            if (!m_seriesVisible[static_cast<size_t>(style.series)])
                continue;

            bool have_prev = false;
            ImVec2 prev{};
            for (int i = 0; i < history_size; ++i)
            {
                const float value = monitor.sample(style.series, i);
                if (!validSample(value))
                {
                    have_prev = false;
                    continue;
                }

                const float x = min.x + (static_cast<float>(i) / static_cast<float>(history_size - 1)) * width;
                const float y = max.y - std::min(value / max_ms, 1.0f) * height;
                ImVec2 point(x, y);
                if (have_prev)
                    draw_list->AddLine(prev, point, style.color, 2.0f);
                prev = point;
                have_prev = true;
            }
        }
    }
    draw_list->PopClipRect();

    ImGui::Spacing();

    if (ImGui::BeginTable("##perf_series", 4, ImGuiTableFlags_SizingStretchProp |
                                            ImGuiTableFlags_RowBg |
                                            ImGuiTableFlags_BordersInnerV))
    {
        ImGui::TableSetupColumn("Series");
        ImGui::TableSetupColumn("Latest");
        ImGui::TableSetupColumn("Visible");
        ImGui::TableSetupColumn("Color");
        ImGui::TableHeadersRow();

        for (const SeriesStyle& style : seriesStyles())
        {
            const size_t index = static_cast<size_t>(style.series);
            const float latest = monitor.latest(style.series);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(style.label);

            ImGui::TableSetColumnIndex(1);
            if (validSample(latest))
                ImGui::Text("%.3f ms", latest);
            else
                ImGui::TextDisabled("--");

            ImGui::TableSetColumnIndex(2);
            std::string checkbox_id = std::string("##visible") + style.label;
            ImGui::Checkbox(checkbox_id.c_str(), &m_seriesVisible[index]);

            ImGui::TableSetColumnIndex(3);
            ImVec4 color = ImGui::ColorConvertU32ToFloat4(style.color);
            std::string color_id = std::string("##color") + style.label;
            ImGui::ColorButton(color_id.c_str(), color,
                               ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
                               ImVec2(18.0f, 18.0f));
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
