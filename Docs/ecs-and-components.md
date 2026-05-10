# ECS & Components

Garden uses [`entt`](https://github.com/skypjack/entt) for the ECS. The single registry your game shares with the engine lives at `g_services->game_world->registry`.

## Entities, components, views

Create an entity, attach components, query:

```cpp
auto& reg = g_services->game_world->registry;

entt::entity e = reg.create();
reg.emplace<TransformComponent>(e, 0.0f, 1.0f, 0.0f);
reg.emplace<TagComponent>(e).name = "Pickup";

// Iterate every entity that has Transform AND Tag
auto view = reg.view<TransformComponent, TagComponent>();
for (auto entity : view) {
    auto& t = view.get<TransformComponent>(entity);
    auto& tag = view.get<TagComponent>(entity);
    /* ... */
}
```

This is plain entt — anything in the entt docs works. The engine never owns "your" entities; the registry is shared.

## Built-in components

These are pre-registered by the engine and editable in the inspector out of the box. You can attach them from C++ or from a level/prefab JSON.

| Component | Purpose |
| :--- | :--- |
| `TagComponent` | Display name (also used by save/load) |
| `TransformComponent` | Position, rotation (Euler degrees), scale |
| `MeshComponent` | Renderable mesh (`shared_ptr<mesh>`) |
| `TerrainComponent` | Heightmap-based terrain |
| `RigidBodyComponent` | Jolt rigid body |
| `Collider*Component` | Box / sphere / capsule / mesh colliders |
| `CharacterControllerComponent` | Capsule character controller |
| `AudioSourceComponent` | Spatial audio source |
| `AnimationComponent` | Skeletal animation player |
| `IKComponent` | Two-bone or FABRIK chain |
| `InputComponent` | Per-entity input bindings |
| `camera` | Active rendering camera |
| `PrefabInstanceComponent` | Marks an entity as instanced from a prefab |

A full reference of fields is the source — `Engine/src/Components/Components.hpp` and the `Engine/src/Components/*.hpp` headers.

## Custom components

The path is: declare the struct, give it a `static reflect(Reflector<T>&)` method, register it in `gardenRegisterComponents`.

### 1. Declare it

```cpp
// src/Components/HealthComponent.hpp
#pragma once
#include "Reflection/Reflector.hpp"
#include <cstdint>

struct HealthComponent
{
    int32_t health     = 100;
    int32_t max_health = 100;
    bool    alive      = true;

    static void reflect(Reflector<HealthComponent>& r)
    {
        r.display("Health").category("Gameplay");

        r.field<&HealthComponent::health>("health")
            .tooltip("Current HP").drag(1.0f).range(0, 9999);

        r.field<&HealthComponent::max_health>("max_health")
            .tooltip("Maximum HP").drag(1.0f).range(1, 9999);

        r.field<&HealthComponent::alive>("alive");
    }
};
```

The reflection layer reads the static `reflect` to know:

- the display name and category in the inspector,
- which fields to show, what tooltip and drag/range each gets,
- how to JSON-serialize the component for levels and prefabs.

### 2. Register it

```cpp
GAME_API void gardenRegisterComponents(ReflectionRegistry* registry)
{
    registry->reflect<HealthComponent>("HealthComponent", "MyGame.dll");
}
```

The second argument is the **source id** — the engine uses it to unregister components belonging to your DLL on unload.

After registration, `HealthComponent` shows up in the inspector's *Add Component* menu, can be saved into prefabs and levels, and round-trips through JSON.

### 3. Reflector field options

Available chainables on a `r.field<...>(...)`:

| Method | Effect |
| :--- | :--- |
| `.tooltip("text")` | Hover text in the inspector |
| `.category("Foo")` | Groups fields into a collapsing header |
| `.drag(step)` | Drag widget step size for numeric fields |
| `.range(min, max)` | Clamp + slider bounds |
| `.assetPath()` | Render as asset picker (drag-drop from Content Browser) |

Available chainables on the `r` itself:

| Method | Effect |
| :--- | :--- |
| `.display("Name")` | Display name (default: registered name) |
| `.category("Group")` | Component category in *Add Component* menu |
| `.removable(false)` | Hide the trash icon (use for components the entity needs) |

## Serialization

Levels and prefabs serialise components by walking the registered fields. You get JSON support for free **only** for fields the reflector understands: ints, floats, bools, strings, `glm::vec2/3/4`, `glm::quat`, enums declared via the reflection helpers, and `std::vector` of those.

For exotic fields (raw pointers, `shared_ptr`, opaque handles), do one of:

- Mark them transient (don't reflect) and rebuild on `gardenOnLevelLoaded`.
- Reflect a string/path you can resolve back to the runtime resource.

`MeshComponent` is the canonical example — its `shared_ptr<mesh>` is **not** reflected; the level stores a model path and the engine reattaches the mesh on load.

## Where to put per-frame logic

`entt` doesn't have systems as first-class objects. Conventions in this codebase:

- A **system** is just a free function (or a class with one method) that takes the registry and a `dt`.
- Call them from `gardenGameUpdate(dt)` in deterministic order.
- Use the [Event Bus](#event-bus) for cross-system communication so systems don't have to know about each other.

```cpp
// src/Systems/HealthRegen.cpp
void tickHealthRegen(entt::registry& reg, float dt)
{
    auto view = reg.view<HealthComponent>();
    for (auto e : view) {
        auto& h = view.get<HealthComponent>(e);
        if (!h.alive) continue;
        h.health = std::min(h.max_health, h.health + int(2.0f * dt));
    }
}
```

```cpp
GAME_API void gardenGameUpdate(float dt)
{
    auto& reg = g_services->game_world->registry;
    tickHealthRegen(reg, dt);
    // ... other systems ...
}
```

## Event Bus

For decoupled communication, use `EventBus` (`Engine/src/Events/EventBus.hpp`) — a thin wrapper over `entt::dispatcher` with both immediate and deferred dispatch. Define your event struct, subscribe in `gardenGameInit`, drain deferred events at a known point in `gardenGameUpdate`.

## Pitfalls

- **Re-registering on hot reload is fine** — `unregisterBySource("MyGame.dll")` is called automatically when the host unloads your DLL.
- **`type_hash<T>` must be unique per component.** Don't have two components named `Health` in different headers — entt will conflate them.
- **Don't capture entity handles into long-lived globals.** A level reload destroys the registry; cache lookups inside `gardenOnLevelLoaded` instead.
