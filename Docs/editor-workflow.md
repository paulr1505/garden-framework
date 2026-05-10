# Editor Workflow

Day-to-day, you'll spend most of your time in `Editor.exe`. This page covers the loop: open project → edit level → enter PIE → iterate.

## Launching

```bat
bin\Editor.exe --project C:\dev\MyGame\MyGame.garden
```

Without `--project`, the project browser opens. Backend flags (`-d3d12`, `-vulkan`, `-metal`) override the saved backend for one launch. Editor settings are stored in `bin/editorconfig.cfg`.

## The panels

Default layout is dockable — drag tabs around, save your layout. Key panels:

| Panel | Use it for |
| :--- | :--- |
| **Viewport** | The 3D scene. Right-click + WASD to fly the editor camera. Gizmos here. |
| **Scene Hierarchy** | Tree of entities. Click to select, right-click for context (save as prefab, delete, duplicate). |
| **Inspector** | Components on the selected entity. Add/remove components. Edit reflected fields. |
| **Content Browser** | Asset tree. Drag `.prefab` to viewport. Right-click to reimport. |
| **Console** | Dev console. Run ConVar commands, read filtered logs. |
| **Toolbar** | Play, Pause, Eject, Save Level. |
| **Level Settings** | Lighting, environment, per-level config. |
| **Physics Debug** | Toggle collider wireframes, AABBs, contact points. |
| **NavMesh** | Generate / save / test pathfinding. |
| **LOD Settings** | Per-mesh LOD thresholds. |
| **Model Preview** | Inline 3D view of a mesh asset. |
| **Status Bar** | Mode (Edit / Play), backend, frame time. |

## Editing a level

1. **File ▸ New Level** or **Open**.
2. Drag prefabs from the Content Browser into the Viewport.
3. Use the gizmos (Q / W / E / R for select / translate / rotate / scale, conventional ImGuizmo shortcuts).
4. Add components to entities in the Inspector.
5. **File ▸ Save**.

Levels are JSON; commit them to source control. Diffs are noisy but readable.

## Play-In-Editor (PIE)

Pressing **Play** on the toolbar:

1. Snapshots the editor's world state.
2. Loads (or hot-loads) your DLL.
3. Calls `gardenOnPlayStart`.
4. Runs your `gardenGameUpdate` until you press Stop.
5. Calls `gardenOnPlayStop` and restores the snapshot.

You can **Eject** mid-play to free-fly the editor camera through a paused world, then re-enter Play. **Pause** stops update ticks but keeps rendering.

### Network PIE

If your DLL exports server hooks, the editor can spawn a server child-process and one or more client child-processes when you press Play. Configure server count and listen port in the toolbar's PIE menu. Each client process is a separate window, simulating real network play.

## Iteration loop

Typical cycle while building a feature:

```
1. Edit C++ in your IDE.
2. Build the game DLL.
3. Stop PIE (if running).
4. Press Play — the editor reloads your DLL.
5. Test.
```

For asset changes (textures, models, levels), the editor picks them up on next level load. For shaders, run `compile_shaders_slang` and restart the editor.

## Common workflows

### Wiring up a new component

1. Add the struct + `static reflect()` in `src/Components/`.
2. Register it in `gardenRegisterComponents`.
3. Build DLL.
4. In the editor, select an entity → Inspector → **Add Component** → your component appears.
5. Edit fields, save level. Component round-trips through JSON automatically.

### Authoring a prefab

1. Build an entity with the components you want.
2. Right-click in Scene Hierarchy → **Save as Prefab**.
3. Place under `assets/prefabs/`.
4. Drag from the Content Browser to spawn instances.

### Debugging a crash on level load

1. Run `Editor.exe` from a terminal so logs go to stdout.
2. Look at the last "Loading entity ..." line — that's where it died.
3. Common causes: missing reflected component (level references a component your DLL didn't register), stale `.cmesh` from a model that no longer exists, prefab path moved.

### Checking a build before shipping

Templates live under `Templates/`. Compare your project's `.buildscript` against the template — drift here is the number-one cause of "works on my machine".

## Editor extensions (plugins)

You can write **editor plugins** that add panels, asset loaders, or tools — separate from your game DLL. They live under `EditorPlugins/` (engine repo) or your project's plugin folder, and are built/registered through GardenCLI:

```bash
garden new-plugin MyPlugin
garden generate-plugin MyPlugin.gardenplugin
```

These are not required to make a game — only to extend the editor itself.
