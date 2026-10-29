# Mellangan

Linux support is coming, possibly macos.

The programming paradigm is "C with classes": that means the game is written in C++20 and might feel like C++ but the code is structured to use C idioms.

The name was a random headbash against the keyboard and stands for: nothing


A yet small top-down "v-survivors"-like game built with C++20 on the [Vulkan2D](https://github.com/soulwax/Vulkan2D) renderer,
using [EnTT](https://github.com/skypjack/entt) for entity storage and [spdlog](https://github.com/gabime/spdlog) for logging.

Survive the night against a horde of enemies blabla, but with the catch that you need to traverse the biomes to find shrines to light and kindle them to win the run. The enemies are a mix of melee and ranged, and the player can pick up upgrades to help them survive.
Enemies can have their own type of attacks, upgrades, passives and abilities.
-
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

On Linux and macOS, `./dist.sh` does the same in bash (`./dist.sh minor`, `./dist.sh none`) and writes
`dist/Mellangan-<version>-<linux|macos>.tar.gz` with a `.sha256` beside it. On macOS it bundles the Vulkan loader and
MoltenVK from `$VULKAN_SDK` into `lib/`; players start the game with `Mellangan.command`.

## Controls

Combat supports four timed afflictions. Burn and poison deal damage every half second; chill slows movement;
marked increases damage from direct hits. Reapplying a status keeps its strongest potency and extends its
remaining duration when the new application lasts longer. Frost Nova chills enemies by 35% for two seconds,
and fire pools leave a short burn when enemies move away. Afflictions clear on death or revival.

| Key | Action |
| --- | --- |
| WASD / Arrows | Move |
| Space | Class skill |
| 1-4, Arrows + Enter, Mouse | Pick an upgrade (4 for a relic) |
| Tab | World map (arrows pan, 1-3 zoom); on the title, switch world mode |
| Up / Down (title) | Recall a past world |
| Left / Right (class screen) | Choose a class |
| C (title, pause) | Codex |
| Esc / P | Pause |
| F11 | Fullscreen |
| F3 | FPS counter |

Run with `--bench` to log performance at 100 to 50,000 enemies (the game quits when it is done).

## Layout

Biome climate fields are stretched to 2.5 times their previous width. Ground materials and vegetation mix over
an eight-tile sampling radius at biome borders, while structures, dirt paths, lava, and oasis water keep their
local shapes. Seeds remain deterministic; this generation change relocates biome boundaries. Old explored
charts retain their explored area and refresh biome colours and structure markers when loaded.

Debug `--test` includes seeded biome-spacing and transition checks. To export a before/after map on Windows:

```powershell
$env:MELLANGAN_BIOME_PREVIEW = 'build/biome-transitions.csv'
./build/bin/Debug/MellanganApp.exe --test
uv run --with pillow tools/biome_transition_preview.py build/biome-transitions.csv build/biome-transitions.png
```

`./build/bin/Debug/MellanganApp.exe --test-biome-view 179 -142 0` opens a temporary rendered boundary view
at tile `(179, -142)` with noise seed `0`, then exits after 20 seconds. It does not load or save player progress.

- `src/SpriteObject.hpp` – root of everything in the world: position, velocity, pixel position and a sprite
- `src/entity` – `SpriteObject` → `Entity` → `Mob` → `Player` / `Enemy` (`Bat`, `Ghost`, `Cyclops`), projectiles, pickups, and `Pool.c`
  (slab allocator for entities)
- `src/entity/enemy/ai` – goal queue AI: `Goal`, the goals (`LookOut`, `Approach`, `Stalk`, `Follow`, `Melee`, `Ranged`,
  `Hide`) and a plan per enemy kind; `src/Workers` runs it on a thread pool
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
- `tools/sprite_kit.py`, `tools/make_sprites.py` – procedural pixel painter and the sprite-kit band (trees, shrubs, rocks, tufts); `--preview` renders a catalogue
- `tools/make_grounds.py` – carpet ground sets (straw, ash, crystal moss, mycelium, lava) for `Carpet` biomes
- `tools/make_flora.py`, `tools/make_desert.py` – add the recoloured and hand-drawn plants, mud, sand and water tiles to `res/atlas.png`
- `tools/make_winter.py` – adds the frosted winter band to `res/atlas.png` (also run by `make_assets.py`)
- `tools/footprint_preview.py` – composes a dumped structure footprint (ground and props) into a preview PNG
- `tools/building_preview.py` – composes Tiny Town tile rows into a preview PNG, used to design buildings outside the game

See `res/CREDITS.md` for asset licenses.
