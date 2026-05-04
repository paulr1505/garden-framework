#pragma once

#include "Graphics/SceneViewport.hpp"

class MetalRenderAPI;

// Metal SceneViewport bridge. Metal already owns per-viewport PIE render
// targets; this wrapper exposes them through the backend-agnostic editor API,
// matching the Vulkan wrapper while leaving Metal's texture ownership in the
// existing render API.
class MetalSceneViewport : public SceneViewport
{
public:
    MetalSceneViewport(MetalRenderAPI* api, int width, int height);
    ~MetalSceneViewport() override;

    int  width()  const override { return m_width; }
    int  height() const override { return m_height; }
    void resize(int width, int height) override;

    uint64_t getOutputTextureID() const override;
    bool     outputsToBackBuffer() const override { return false; }

    int pieId() const { return m_pie_id; }

private:
    MetalRenderAPI* m_api = nullptr;
    int m_pie_id = -1;
    int m_width  = 0;
    int m_height = 0;
};
