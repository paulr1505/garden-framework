#pragma once

#include "EngineExport.h"

#include <memory>
#include <string>

class mesh;
struct TerrainComponent;

namespace Assets
{
    struct TerrainBuildResult
    {
        std::shared_ptr<mesh> render_mesh;
        std::shared_ptr<mesh> collision_mesh;
        int heightmap_width = 0;
        int heightmap_height = 0;
        bool success = false;
        std::string error_message;
    };

    class ENGINE_API TerrainBuilder
    {
    public:
        static TerrainBuildResult buildFromHeightmap(const TerrainComponent& terrain,
                                                     const std::string& resolved_heightmap_path,
                                                     bool render_with_gpu_displacement);
    };
}
