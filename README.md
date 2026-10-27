# Mellangan

A small top-down survivors-like built with C++20 on the [Vulkan2D](https://github.com/soulwax/Vulkan2D) renderer,
using [EnTT](https://github.com/skypjack/entt) for entity storage and [spdlog](https://github.com/gabime/spdlog) for logging.

Walk; your weapons fire on their own. Collect gems to level up and choose upgrades. Each run is one night: kindle the
shrines scattered through the biomes to pull the dawn closer, and survive the horde until it breaks.

## Building

Requires CMake 3.21+, a C++20 compiler (MSVC on Windows, GCC 11+ or Clang 14+ on Linux), and the
[Vulkan SDK](https://vulkan.lunarg.com/) for its `slangc` shader compiler. SDL is built from the submodule and linked
statically.

```sh
git clone --recurse-submodules git@github.com:soulwax/Mellangan.git
cmake -S . -B build
cmake --build build --config Release
```

Shaders in `res/shaders` are compiled to SPIR-V at build time with the SDK's `slangc`, so Vulkan2D's
runtime Slang compiler is disabled and never built. The executable lands next to a copy of `res/`.

### Windows

If `VULKAN_SDK` is not set, CMake picks the newest install under `C:/VulkanSDK`. The executable is
`build/Release/Mellangan.exe`.

### Linux

Install a compiler, CMake, Ninja or Make, and SDL's build dependencies for X11, Wayland and audio. On Debian or Ubuntu:

```sh
sudo apt install build-essential cmake ninja-build pkg-config libx11-dev libxext-dev libxrandr-dev libxcursor-dev   libxi-dev libxss-dev libxtst-dev libwayland-dev libxkbcommon-dev wayland-protocols libegl-dev   libasound2-dev libpulse-dev libpipewire-0.3-dev libdbus-1-dev libudev-dev libvulkan-dev
```

On Arch: `sudo pacman -S base-devel cmake ninja libx11 libxext libxrandr libxcursor libxi libxss libxtst wayland
libxkbcommon wayland-protocols alsa-lib libpulse pipewire dbus systemd-libs vulkan-icd-loader vulkan-headers`.

Download the LunarG Vulkan SDK tarball for Linux, extract it, and source its environment so `VULKAN_SDK` and
`slangc` are set before configuring:

```sh
source ~/vulkansdk/<version>/setup-env.sh
cmake -S . -B build -G Ninja
cmake --build build
./build/Mellangan
```

Single-config generators default to a Release build. Players need a Vulkan driver (Mesa or the GPU vendor's).
Crashes log a backtrace to `mellangan.log` next to the executable.

## Releasing

The version lives in `VERSION` (`MAJOR.MINOR.PATCH`) and every release needs notes under `## [Unreleased]` in
`CHANGELOG.md`. Then run, on Windows or Linux, with PowerShell 7 (`pwsh`; on Windows 7-Zip must be on PATH or
installed):

```powershell
./dist.ps1             # patch bump: 0.1.0 -> 0.1.1
./dist.ps1 -Bump minor # 0.1.1 -> 0.2.0
./dist.ps1 -Bump none  # rebuild the current version, changelog untouched
```

It bumps `VERSION`, dates the changelog section, builds Release, and writes `dist/Mellangan-<version>-windows.7z` or
`dist/Mellangan-<version>-linux.tar.gz` containing the executable, `res/`, and the changelog. To package the other
platform's build of the same version, run `./dist.ps1 -Bump none` there. SDL and, on Windows, the CRT are linked
statically, so players only need a Vulkan-capable driver.
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

- `src/SpriteObject.hpp` – root of everything in the world: position, velocity, pixel position and a sprite
- `src/entity` – `SpriteObject` → `Entity` → `Mob` → `Player` / `Enemy` (`Bat`, `Ghost`, `Cyclops`), projectiles, pickups, and `Pool.c`
  (slab allocator for entities)
- `src/particle` – particles kept outside the registry in `Level::particles`: `Spark` and `TextParticle`, and `Debris` after dittany: blood, meat, smoke, flames, heal crosses and
  the `Explosion` emitter
- `src/item` – `Upgrade` and its weapons and passives
- `src/level` – `Level` (backed by an `entt::registry`) and the `Spawner`; world generation as small table-driven
  modules: `Noise.c` (pure C Perlin/fBm), `Biome`, `Feature` (scattered props), `Structure` (layouts) and
  `Encounter` (what a structure does when entered)
- `src/menu` – title, pause, level-up, and death menus
- `src/gfx` – `Screen`, the thin layer over Vulkan2D
- `res/shaders` – `post.slang` (lighting composite, bloom, tonemap, damage feedback) and `flash.slang` (hit flash)
- `tools/make_assets.py` – rebuilds `res/atlas.png` from the Kenney sheet plus generated FX sprites

See `res/CREDITS.md` for asset licenses.
