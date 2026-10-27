# Mellangan

A small top-down survivors-like built with C++20 on the [Vulkan2D](https://github.com/soulwax/Vulkan2D) renderer,
using [EnTT](https://github.com/skypjack/entt) for entity storage and [spdlog](https://github.com/gabime/spdlog) for logging.

Walk; your weapons fire on their own. Collect gems to level up, choose upgrades, and survive the waves and the cyclops.

## Building

Requires CMake 3.21+, a C++20 compiler (MSVC on Windows), and the [Vulkan SDK](https://vulkan.lunarg.com/).

```sh
git clone --recurse-submodules git@github.com:soulwax/Mellangan.git
cmake -S . -B build
cmake --build build --config Release
```

On Windows, if `VULKAN_SDK` is not set, CMake picks the newest install under `C:/VulkanSDK`.
Shaders in `res/shaders` are compiled to SPIR-V at build time with the SDK's `slangc`, so Vulkan2D's
runtime Slang compiler is disabled and never built.
The executable lands in `build/Release/` with `res/` copied next to it.

## Releasing

The version lives in `VERSION` (`MAJOR.MINOR.PATCH`) and every release needs notes under `## [Unreleased]` in
`CHANGELOG.md`. Then run (PowerShell 7, 7-Zip on PATH or installed):

```powershell
./dist.ps1             # patch bump: 0.1.0 -> 0.1.1
./dist.ps1 -Bump minor # 0.1.1 -> 0.2.0
./dist.ps1 -Bump none  # rebuild the current version, changelog untouched
```

It bumps `VERSION`, dates the changelog section, builds Release, and writes `dist/Mellangan-<version>.7z` containing
the executable, `res/`, and the changelog. The CRT is linked statically, so players only need a Vulkan-capable driver.
If the build fails, `VERSION` and `CHANGELOG.md` are restored.

## Controls

| Key | Action |
| --- | --- |
| WASD / Arrows | Move |
| 1-3, Arrows + Enter, Mouse | Pick an upgrade |
| Esc / P | Pause |
| F11 | Fullscreen |
| F3 | FPS counter |

Run with `--bench` to log performance at 100 to 50,000 enemies (the game quits when it is done).

## Layout

- `src/entity` – `Entity` → `Mob` → `Player` / `Enemy` (`Bat`, `Ghost`, `Cyclops`), projectiles, pickups, particles
- `src/item` – `Upgrade` and its weapons and passives
- `src/level` – `Level` (backed by an `entt::registry`) and the `Spawner`; world generation as small table-driven
  modules: `Noise.c` (pure C Perlin/fBm), `Biome`, `Feature` (scattered props), `Structure` (layouts) and
  `Encounter` (what a structure does when entered)
- `src/menu` – title, pause, level-up, and death menus
- `src/gfx` – `Screen`, the thin layer over Vulkan2D
- `res/shaders` – `post.slang` (lighting composite, bloom, tonemap, damage feedback) and `flash.slang` (hit flash)
- `tools/make_assets.py` – rebuilds `res/atlas.png` from the Kenney sheet plus generated FX sprites

See `res/CREDITS.md` for asset licenses.
