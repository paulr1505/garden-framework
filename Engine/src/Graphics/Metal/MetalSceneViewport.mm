#include "MetalSceneViewport.hpp"
#include "MetalRenderAPI.hpp"

MetalSceneViewport::MetalSceneViewport(MetalRenderAPI* api, int width, int height)
    : m_api(api)
    , m_width(width > 0 ? width : 1)
    , m_height(height > 0 ? height : 1)
{
    if (m_api)
        m_pie_id = m_api->createPIEViewport(m_width, m_height);
}

MetalSceneViewport::~MetalSceneViewport()
{
    if (m_api && m_pie_id >= 0)
        m_api->destroyPIEViewport(m_pie_id);
}

void MetalSceneViewport::resize(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    if (width == m_width && height == m_height) return;

    m_width = width;
    m_height = height;

    if (m_api && m_pie_id >= 0)
        m_api->setPIEViewportSize(m_pie_id, width, height);
}

uint64_t MetalSceneViewport::getOutputTextureID() const
{
    if (!m_api || m_pie_id < 0) return 0;
    return m_api->getPIEViewportTextureID(m_pie_id);
}
