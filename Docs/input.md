# Input

Garden uses SDL3 for input. The host pumps SDL each frame and exposes per-frame state through `InputManager`. You read it from `g_services->input_manager`.

## Per-frame state

```cpp
#include "InputManager.hpp"
#include <SDL3/SDL.h>

void gardenGameUpdate(float dt)
{
    auto* in = g_services->input_manager;

    // Edge-triggered: true only on the frame the key went down
    if (in->wasKeyPressed(SDL_SCANCODE_SPACE)) jump();

    // Level-triggered: true while held
    if (in->isKeyDown(SDL_SCANCODE_W)) moveForward(dt);

    // Edge-triggered release
    if (in->wasKeyReleased(SDL_SCANCODE_E)) endInteract();

    // Mouse delta (raw, in pixels — apply your own sensitivity)
    glm::vec2 dm = in->getMouseDelta();
    yaw   -= dm.x * mouse_sens;
    pitch -= dm.y * mouse_sens;

    // Mouse buttons
    if (in->wasMouseButtonPressed(SDL_BUTTON_LEFT)) shoot();
}
```

Use scancodes (`SDL_SCANCODE_*`), not keycodes — scancodes are layout-independent. `Engine/src/InputManager.hpp` is the source of truth for the available methods.

## Action mappings

For anything beyond a prototype, map physical inputs to *actions* and read actions in your gameplay code. This survives keybind changes without you rewriting the gameplay.

The pattern in the FPSShooter template (`src/InputHandler.hpp` in the engine):

1. Define an enum of actions: `MoveForward`, `Jump`, `Fire`, `Reload`, ...
2. Map each action to a scancode or button at startup.
3. Gameplay reads `actions.isPressed(Action::Jump)` instead of `SDL_SCANCODE_SPACE`.

If you want a configurable rebind UI, persist the mapping with the [ConVar system](console-and-convars.md) or your own JSON next to the `.garden` file.

## Mouse capture

For an FPS-style mouselook you usually want the OS cursor hidden and locked to the window. The `Application` exposes the SDL window through `g_services->application` — set relative-mouse mode via SDL when entering gameplay and clear it when opening menus / pausing.

## Per-entity input components

`InputComponent` (in `Engine/src/Components/InputComponent.hpp`) lets you attach input-driven behaviour to entities through reflection (so it shows up in the inspector). Useful when you want designers to wire keys to entities in the editor instead of in code.

For tightly coupled gameplay (e.g., the local player), reading `InputManager` directly in `gardenGameUpdate` is simpler and what the templates do.

## What does *not* go through InputManager

- **Editor UI input** — handled by ImGui. If a menu is hovered/focused, your gameplay should ignore input that frame. Check `ImGui::GetIO().WantCaptureKeyboard` / `WantCaptureMouse`.
- **HUD input** — RmlUi handles its own focus. Either gate gameplay input on `RmlUiManager` having focus, or design your HUD to be read-only during play.
- **Network input** — the FPSShooter template wraps local input in `MovementInput` / `InputState` structs and ships them to the server. The server doesn't read SDL.
