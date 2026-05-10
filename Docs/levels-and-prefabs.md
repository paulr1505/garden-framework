# Levels & Prefabs

Levels are the persistent state of a scene: which entities exist, their components, and any per-level settings. Prefabs are reusable entity templates you spawn from levels or code.

## Levels

Levels are JSON files under `assets/levels/`, conventionally named `*.level.json`. The default level is set in your `.garden` file:

```json
"default_level": "assets/levels/main.level.json"
```

You almost never write level JSON by hand. The editor's **File ▸ New Level / Save / Save As** writes it for you.

### Loading a level from C++

Use `LevelManager` from `EngineServices`:

```cpp
g_services->level_manager->loadLevel("assets/levels/level2.level.json");
```

`gardenOnLevelLoaded` fires after deserialisation completes — that's where to do your per-level setup (find the player entity, snap the camera, start music).

### Scene transitions

For game-flow transitions (menu → match → results), use the same `LevelManager` calls. The host handles teardown of physics and the previous registry. Persistent data — score, inventory — should live in your DLL, not in the level.

### What gets saved

Per entity:
- `TagComponent` (the entity's display name)
- `TransformComponent`
- Any reflected component you registered, with its reflected fields
- The mesh path and rendering flags from `MeshComponent`
- Collider/rigid-body data from physics components
- Prefab origin (so prefab instances know where they came from)

Things **not** saved automatically:
- Runtime-only fields you didn't reflect (`shared_ptr`, raw pointers, etc.)
- Networked replicated state — the server is authoritative; level JSON describes the static scene, not gameplay state

## Prefabs

A prefab captures one entity's components, mesh, and collider in a `.prefab` JSON file you can spawn many times. Prefabs support nesting (a prefab can contain other prefab instances) and hot-reload (changes update existing instances).

### Authoring a prefab in the editor

1. Build the entity in the scene (transform, mesh, components).
2. Right-click in the Scene Hierarchy and choose **Save as Prefab**.
3. Pick a path under `assets/prefabs/`.

The Content Browser shows `.prefab` files in purple with a puzzle-piece icon.

### Spawning a prefab in the editor

- **Drag-drop** a `.prefab` from the Content Browser onto the Viewport.
- **Double-click** in the Content Browser to spawn at origin.

The spawned entity gets a `PrefabInstanceComponent` recording which prefab it came from.

### Spawning a prefab from C++

```cpp
#include "Prefab/PrefabManager.hpp"

auto& reg = g_services->game_world->registry;

// Spawn at the prefab's stored transform
entt::entity e1 = PrefabManager::get().spawn(reg, "assets/prefabs/enemy.prefab");

// Spawn at a specific position
entt::entity e2 = PrefabManager::get().spawnAt(
    reg, "assets/prefabs/enemy.prefab", 10.0f, 0.0f, 5.0f);
```

`PrefabManager::get()` is a host-side singleton — it's already initialised by the time `gardenGameInit` runs. Don't construct your own.

### Modifying spawned instances

After spawn, the entity is a normal entt entity. Edit components directly:

```cpp
auto e = PrefabManager::get().spawnAt(reg, "assets/prefabs/enemy.prefab", x, y, z);
reg.get<TagComponent>(e).name = "Boss";
reg.emplace<HealthComponent>(e, 500);
```

The prefab serves as the **template**; per-instance overrides are ordinary component edits.

### Prefab JSON shape

Roughly:

```json
{
    "format": "garden_prefab",
    "version": 1,
    "name": "Enemy Soldier",
    "components": {
        "TagComponent": { "name": "Enemy Soldier" },
        "TransformComponent": { "position": [0,0,0], "rotation": [0,0,0], "scale": [1,1,1] },
        "RigidBodyComponent": { "mass": 1.0, "apply_gravity": true }
    },
    "mesh": { "path": "assets/models/enemy.glb", "culling": true }
}
```

For details, see `Documentation/prefabs.md` (player-facing) and `Documentation/developers/prefab-internals.md` (engine side).

## Patterns

- **Spawning waves**: keep a list of prefab paths in C++, iterate, `spawnAt` with offsets. Don't bake spawn positions into prefabs.
- **Static scenery**: place in the level. Doesn't need a prefab.
- **Recurring entities** (NPCs, pickups, projectiles): make a prefab. Saves you re-authoring the components and rigid-body settings every time.
- **Player**: usually placed once in the level. The `FPSShooter` template finds the placed player in `gardenOnLevelLoaded`.
