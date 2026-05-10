# Audio

Garden uses [miniaudio](https://miniaud.io/) wrapped by `AudioSystem`. The host owns one `AudioSystem`; game code interacts with it through the system header and through `AudioSourceComponent` on entities.

```cpp
#include "Audio/AudioSystem.hpp"
```

## Audio groups

Audio routes through four groups, each with its own volume:

| Group | Use for |
| :--- | :--- |
| `SFX` | Gameplay sound effects |
| `Music` | Background music |
| `Voice` | Dialogue, VO |
| `UI` | Menu clicks, HUD beeps |

Set group volumes from C++ or expose them via ConVars so players can adjust:

```cpp
AudioSystem::get().setGroupVolume(AudioGroup::Music, 0.4f);
AudioSystem::get().setGroupVolume(AudioGroup::SFX,   0.8f);
```

## One-shot sounds

For HUD beeps and short SFX where you don't care about the source position:

```cpp
AudioSystem::get().playOneShot("assets/audio/click.wav", AudioGroup::UI);
```

## Spatial audio (3D positional)

Attach `AudioSourceComponent` to an entity. The system reads the entity's `TransformComponent.position` and the active camera each frame to compute attenuation and panning.

```cpp
auto& src = reg.emplace<AudioSourceComponent>(e);
src.path        = "assets/audio/engine_loop.wav";
src.group       = AudioGroup::SFX;
src.looping     = true;
src.volume      = 1.0f;
src.min_distance = 1.0f;
src.max_distance = 30.0f;
src.autoplay    = true;
```

Field names — and any extras — live in `Engine/src/Components/AudioSourceComponent.hpp`. The component is reflected, so you can also author audio sources in the editor with no code.

## Listener position

The listener follows the active rendering camera (`world::world_camera`). You don't position the listener explicitly — moving the camera moves the listener.

In the editor (out of PIE) the listener is the editor camera, which can be a useful debug surface — you hear sounds from where you're looking.

## Asset formats

miniaudio decodes WAV, MP3, FLAC, and Vorbis. WAV is fastest to load and is what the templates use.

## Patterns

- **Footstep system**: subscribe to a `FootstepEvent` on the event bus, look up the surface material from the raycast, call `playOneShot` with the right clip.
- **Dynamic music**: keep two `AudioSourceComponent`-bearing entities pinned to the camera, crossfade their volumes when combat state changes.
- **Stopping a looping source**: remove `AudioSourceComponent` or set `volume = 0`. The system stops playback when the component is destroyed.
