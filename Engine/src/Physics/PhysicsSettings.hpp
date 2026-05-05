#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

#include <cstdint>
#include <glm/glm.hpp>

namespace PhysicsObjectLayers
{
    static constexpr JPH::ObjectLayer Static = 0;
    static constexpr JPH::ObjectLayer Dynamic = 1;
    static constexpr JPH::ObjectLayer Count = 2;
}

namespace PhysicsBroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer Static(0);
    static constexpr JPH::BroadPhaseLayer Dynamic(1);
    static constexpr uint32_t Count = 2;
}

struct PhysicsLayerSettings
{
    JPH::ObjectLayer static_body = PhysicsObjectLayers::Static;
    JPH::ObjectLayer dynamic_body = PhysicsObjectLayers::Dynamic;
    JPH::ObjectLayer object_layer_count = PhysicsObjectLayers::Count;

    JPH::BroadPhaseLayer static_broad_phase = PhysicsBroadPhaseLayers::Static;
    JPH::BroadPhaseLayer dynamic_broad_phase = PhysicsBroadPhaseLayers::Dynamic;
    uint32_t broad_phase_layer_count = PhysicsBroadPhaseLayers::Count;

    bool shouldObjectCollideWithBroadPhase(JPH::ObjectLayer object_layer, JPH::BroadPhaseLayer broad_phase_layer) const
    {
        if (object_layer == static_body)
            return broad_phase_layer == dynamic_broad_phase;
        if (object_layer == dynamic_body)
            return true;
        return false;
    }

    bool shouldObjectsCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const
    {
        if (a == static_body)
            return b == dynamic_body;
        if (a == dynamic_body)
            return true;
        return false;
    }

    JPH::BroadPhaseLayer broadPhaseForObjectLayer(JPH::ObjectLayer object_layer) const
    {
        if (object_layer == static_body)
            return static_broad_phase;
        if (object_layer == dynamic_body)
            return dynamic_broad_phase;
        return dynamic_broad_phase;
    }
};

struct PhysicsSystemSettings
{
    PhysicsLayerSettings layers;

    glm::vec3 gravity_direction = glm::vec3(0.0f, -1.0f, 0.0f);
    float gravity_acceleration = 9.81f;
    float fixed_delta = 1.0f / 60.0f;

    uint32_t max_bodies = 1024;
    uint32_t body_mutex_count = 0;
    uint32_t max_body_pairs = 1024;
    uint32_t max_contact_constraints = 1024;
    uint32_t temp_allocator_size_bytes = 10u * 1024u * 1024u;
    uint32_t worker_thread_count = 0; // 0 means auto.
    uint32_t collision_steps = 1;

    float max_body_velocity = 100.0f;
    float max_character_velocity = 100.0f;
    float min_body_mass = 0.001f;
    float dynamic_linear_damping = 0.0f;
    float kinematic_gravity_factor = 0.0f;
    float force_epsilon = 0.001f;

    float mesh_degenerate_triangle_epsilon = 1.0e-12f;
    float player_ground_probe_padding = 0.3f;
    float player_min_ground_normal_y = 0.7f;
    float raycast_direction_epsilon = 0.0001f;
};
