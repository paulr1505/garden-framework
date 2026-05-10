# Console & ConVars

Garden has a Source Engine-style **ConVar** system: typed, named values that can be edited at runtime through the developer console, archived to disk, and (optionally) replicated from server to clients.

```cpp
#include "Console/ConVar.hpp"
```

## Declaring a ConVar

ConVars are static — declare them at file scope in your DLL:

```cpp
static ConVar cv_player_speed("player_speed", 6.0f,
    "Player ground move speed (m/s)",
    ConVarFlags::ARCHIVE);

static ConVar cv_god_mode("god_mode", false,
    "Disable player damage",
    ConVarFlags::CHEAT);

static ConVar cv_team_size("team_size", 5,
    "Players per team",
    ConVarFlags::REPLICATED);
```

Read them by calling the typed getter (the API is in `Console/ConVar.hpp` — `getFloat()`, `getBool()`, `getInt()`, `getString()`, plus typed value access). Use ConVars for any value designers/players might want to tweak without recompiling.

## Flags

| Flag | Behavior |
| :--- | :--- |
| `ARCHIVE` | Saved to `bin/config.cfg` (client) / `editorconfig.cfg` (editor) and reloaded on next launch |
| `CHEAT` | Cannot be set from the console unless cheats are enabled |
| `REPLICATED` | Server pushes the value to all connected clients |

Combine with `|`. A typical gameplay tunable is `ARCHIVE | REPLICATED` so the server's value wins and persists.

## The dev console

The editor has a built-in **Console panel** with command input, tab completion, and log filtering. Open it from the *View* menu. From there:

```
> player_speed 8.0          # set
> player_speed              # print current value
> god_mode 1                # blocked unless cheats are enabled
> help player_speed         # description + flags
> exec autoexec.cfg         # run a script of console commands
```

In the standalone client (`Game.exe`) the console is bindable but not always on-screen — the FPS template binds it to the backtick key.

## Bounds

Add bounds when registering and the system clamps + validates:

```cpp
static ConVar cv_fov("fov", 90.0f, "Vertical field of view, degrees",
    ConVarFlags::ARCHIVE, /*min=*/60.0f, /*max=*/120.0f);
```

(Exact bounds-API arguments are in `ConVar.hpp` — pass min/max in whichever overload your version exposes.)

## Custom commands

Beyond simple values, you can register console **commands** — callables invoked from the console:

```cpp
static ConCommand cc_giveweapon("giveweapon",
    "Give the local player a weapon by name",
    [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            LOG_ENGINE_INFO("Usage: giveweapon <rifle|shotgun>");
            return;
        }
        giveWeaponByName(args[1]);
    });
```

`ConCommand` registration is in the same header. Use it for cheats, debug toggles, level reloads — anything imperative.

## Persistence

ConVars with `ARCHIVE` are written next to the executable. The host loads them after `gardenGameInit` runs. If you read an archived ConVar in `gardenGameInit`, it has its archived value already.

Player-facing settings (key bindings, audio levels, video options) are usually best modeled as ConVars. Save once, restore once, expose to a settings menu by name.

## Replication

Server-side, declare `cv_x("...", default, "...", REPLICATED)`. The server publishes the value to clients on connect and on change. Clients see the server's value through the same `cv_x` reference — they don't override it locally.

## Pitfalls

- **Don't hold onto a `ConVar*` long-term.** ConVars live in a global registry; just keep the static.
- **Cheat protection is intent-based.** `CHEAT` keeps the console honest; it doesn't stop a determined player from patching a binary. For competitive games, gate cheaty features on server authority.
- **Naming conflicts** silently clobber. Keep your DLL's ConVars prefixed (`mygame_`, `g_`, `cl_`, `sv_`) to avoid colliding with engine ones.
