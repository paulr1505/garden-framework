# UI

Garden has two UI stacks. They serve different purposes and you'll usually use both.

| Stack | Purpose | Where it renders |
| :--- | :--- | :--- |
| **RmlUi** | In-game HUD, menus, anything player-facing | Composited over the scene by the renderer |
| **ImGui** | Editor and debug UI | Editor only (also debug overlays in Game.exe) |

## RmlUi (game HUD)

[RmlUi](https://github.com/mikke89/RmlUi) is HTML + CSS for games. You author `.rml` (HTML-like) and `.rcss` (CSS) files under `assets/ui/`, load them in your DLL, and the engine draws them via per-backend renderers (D3D12 / Vulkan / Metal).

### Loading a document

```cpp
#include "UI/RmlUiManager.h"

// In gardenGameInit, after services are cached:
auto* rml = RmlUiManager::get();
rml->loadFontFace("assets/ui/fonts/Inter-Regular.ttf");

auto* doc = rml->loadDocument("assets/ui/hud.rml");
doc->Show();
```

### Updating values from C++

Look up elements by ID and set inner text or attributes:

```cpp
auto* health_el = doc->GetElementById("health");
health_el->SetInnerRML(std::to_string(player_health));
```

For larger HUDs use Rml's data binding — bind a C++ struct once and update its fields, the HUD reflects the changes automatically. The FPSShooter template's `GameHUD.cpp` is a working reference.

### Input

RmlUi handles its own input when a document has focus. Gate gameplay input on whether a menu is open:

```cpp
if (rml->isMenuOpen()) {
    // Menu absorbs input this frame
    return;
}
```

### Per-backend renderers

Each backend has its own RmlUi renderer (`Engine/src/UI/Rml*`). You don't pick one — the engine matches the active `IRenderAPI`. Custom shaders for UI effects are not supported through Rml directly; build them as a separate render pass if needed.

## ImGui (debug + editor)

ImGui (docking branch) is wired up by the editor. In your game DLL you can call ImGui inside `gardenGameUpdate` to add debug windows:

```cpp
#include <imgui.h>

void gardenGameUpdate(float dt)
{
    if (ImGui::Begin("Player Debug")) {
        ImGui::Text("Health: %d", g_local_health);
        ImGui::DragFloat("Move speed", &g_move_speed, 0.1f, 0.0f, 50.0f);
    }
    ImGui::End();
}
```

ImGui windows render only when an ImGui-aware host is rendering them — the editor and Game.exe do, the dedicated server does not. Don't put gameplay-critical UI in ImGui; ship it in RmlUi.

### Gizmos

ImGuizmo (translate/rotate/scale handles) is an editor concern. You typically don't draw gizmos from your DLL during play. The editor's panels handle this for you.

## Picking the right tool

- HUD bar / score / kill feed / settings menu → **RmlUi**
- Cheat menu / live tweakables / debug overlays → **ImGui**
- 3D-positioned text labels above NPCs → use the renderer's debug text or build a billboarded mesh; both UI stacks are screen-space.

## Pitfalls

- **One menu open at a time.** Two RmlUi documents claiming focus will fight each other for input.
- **Don't allocate Rml elements per frame.** Build the document once, update text/attributes by ID.
- **ImGui IDs collide.** If two windows have the same title, ImGui merges them. Use `ImGui::PushID(entity)` when drawing per-entity widgets.
