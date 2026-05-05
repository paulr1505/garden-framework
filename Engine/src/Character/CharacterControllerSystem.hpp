#pragma once

#include "Character/CharacterController.hpp"
#include "EngineExport.h"
#include "Physics/PhysicsSettings.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>

namespace JPH
{
    class CharacterVsCharacterCollisionSimple;
    class PhysicsSystem;
    class TempAllocator;
}

class ENGINE_API CharacterControllerSystem
{
public:
    using BodyEntityMap = std::unordered_map<JPH::BodyID, entt::entity>;

    CharacterControllerSystem();
    ~CharacterControllerSystem();

    CharacterControllerSystem(const CharacterControllerSystem&) = delete;
    CharacterControllerSystem& operator=(const CharacterControllerSystem&) = delete;
    CharacterControllerSystem(CharacterControllerSystem&&) noexcept;
    CharacterControllerSystem& operator=(CharacterControllerSystem&&) noexcept;

    void shutdown(BodyEntityMap& body_to_entity);

    void copyPlayerSettings(entt::registry& registry, entt::entity entity);

    JPH::BodyID create(entt::registry& registry,
                       entt::entity entity,
                       JPH::PhysicsSystem& physics_system,
                       JPH::TempAllocator& temp_allocator,
                       BodyEntityMap& body_to_entity,
                       const PhysicsLayerSettings& layer_settings);
    void remove(entt::entity entity, BodyEntityMap& body_to_entity);
    bool has(entt::entity entity) const;

    CharacterControllerState getState(entt::registry& registry, entt::entity entity) const;
    bool setState(entt::registry& registry,
                  entt::entity entity,
                  const CharacterControllerState& state,
                  JPH::PhysicsSystem& physics_system,
                  JPH::TempAllocator& temp_allocator,
                  const PhysicsLayerSettings& layer_settings);
    bool teleport(entt::registry& registry,
                  entt::entity entity,
                  const glm::vec3& position,
                  JPH::PhysicsSystem& physics_system,
                  JPH::TempAllocator& temp_allocator,
                  const PhysicsLayerSettings& layer_settings);
    bool refresh(entt::registry& registry,
                 entt::entity entity,
                 JPH::PhysicsSystem& physics_system,
                 JPH::TempAllocator& temp_allocator,
                 const PhysicsLayerSettings& layer_settings);

    CharacterControllerState simulate(entt::registry& registry,
                                      entt::entity entity,
                                      const CharacterMoveInput& input,
                                      float delta_time,
                                      const PhysicsSystemSettings& settings,
                                      JPH::PhysicsSystem& physics_system,
                                      JPH::TempAllocator& temp_allocator,
                                      BodyEntityMap& body_to_entity);

private:
    struct Runtime;

    std::unordered_map<entt::entity, std::unique_ptr<Runtime>> entity_to_character;
    std::unique_ptr<JPH::CharacterVsCharacterCollisionSimple> character_vs_character_collision;
};
