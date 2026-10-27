# Changelog

All notable changes to Mellangan are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow [Semantic Versioning](https://semver.org/).
`dist.ps1` turns the `Unreleased` section into the next version when it packages a release.

## [Unreleased]

### Added

- Linux support: builds with GCC or Clang through CMake, logs a backtrace on crashes, and packages as
  `Mellangan-<version>-linux.tar.gz` from `dist.ps1` under PowerShell 7.

### Fixed

- Windows releases no longer need `SDL3.dll`: SDL is now linked statically. Before, the game only started where a
  copy happened to be on PATH, such as from the Vulkan SDK.

## [0.2.2] - 2026-10-27

### Changed

- Bigger hordes run faster: 30,000 enemies now hold 59 fps (was 41) and 50,000 run at 22 fps (was 4). Removed
  entities are queued as they die instead of found by scanning everything each tick, each touching pair of enemies is
  separated once instead of twice, enemies far off screen separate on alternate ticks, the spatial grid computes each
  enemy's cell once, and off-screen lights are culled before their draw call.
- `--bench` now also keeps particles at their cap with gore, sparks and explosions, and logs a per-system breakdown
  (entities, particles, housekeeping, light, floor, sort, draw) from a small C profiler.

## [0.2.1] - 2026-10-27

### Changed

- `SpriteObject` is the new root of everything drawn in the world: position, velocity, pixel position and a sprite.
  `Entity` builds on it with the registry id, and props and particles are plain sprite objects.
- Particles no longer live in the EnTT registry. The level keeps them in their own list, ticks and compacts it once
  per tick and draws them in the same depth-sorted pass, and per-kind live counters replace registry counts for the
  particle caps.

## [0.2.0] - 2026-10-27

### Added

- Kindle the Night: a run is one night, and dawn at 15:00 wins it. Stand six seconds in a cold shrine to kindle it:
  it burns for the rest of the night, pulls dawn a minute closer, widens your own light and unleashes a starfire
  blast, and the horde answers with a ring of its strongest unlocked enemies.
- A compass at the edge of the view points to the nearest cold shrine with its distance in tiles. The HUD shows the
  time to dawn, shrines lit and kindling progress.
- The sky warms toward dawn over the last minutes. At dawn the horde burns away and the run ends in victory.
- Cast shadows: your lantern, campfires, cottage windows and kindled shrines throw soft shadows from trees, walls,
  props and walking enemies, stretched away from the flame, longer the farther the light, and breathing with it.
- Debris particles after dittany: blood, meat chunks that trail blood and splash when they land, smoke, flames
  that burn down and leave smoke, and heal crosses, all with 3D gravity, drag, bounce and ground friction, bouncing
  off solid props. Enemies die in gore (flyers in smoke), bombs, novas and the cyclops explode in flame, potions and
  kindled shrines release crosses, and dawn burns the horde to embers.
- The wizard carries a torch: flames lick off its tip and trail behind when you run, and your light and its
  shadows now come from the torch in your hand. Kindled shrines burn with a living flame above their crystal.
- Fire light flickers organically with layered Perlin noise and burns yellower when bright, redder when dim.

### Changed

- The night is darker and the dawn a dimmer, warmer sunrise instead of a washed-out glare.

## [0.1.4] - 2026-10-27

### Added

- `--bench` runs a stress benchmark: fresh runs with 100, 1,000, 10,000, 20,000, 30,000, 40,000 and 50,000
  enemies, logging fps, tick and render cost for each, then quits.

### Changed

- Huge hordes playable
- The depth sort is a three-pass radix sort over packed layer and y keys, and entities are allocated from a C slab

## [0.1.3] - 2026-10-27

### Added

- Each biome favours its own enemies once they unlock: rats, slimes and knights in meadows, bats and spiders in
  forests, slimes and demons in autumn woods rats and scorpions on the heath, bats, ghosts and cultists in barrows
- Structures react the first time you step inside: cottages leave a healing potion, camps a supply barrel, shrines a
  starfire vial; ruins a chest or a gem magnet, and graveyards raise a ring of ghosts.
- Collision detection w/ world: tree stems, bushes, graves, etc.
  block the player and ground enemies, that slide around them. Only the foot of a prop should be solid, so you can walk
  behind crowns, roofs and walls, and through a ruin's archway. Flying enemies ignore that.

### Changed

- World queries cost about a fifth of what they did (0.24 to 0.045 ms per frame) with an identical world: a per-tile
  memo packs biome, dirt, shade and clump into one direct-mapped slot, Perlin gradients come from a table instead of
  a branch, and dirt rims come from a 4-bit neighbour mask.
- Dev F9 warps into the nearest structure you haven't visited yet, so repeated presses "tour the map", and the F8 overlay
  targets much of the same.

## [0.1.2] - 2026-10-27

### Added

- Biomes picked from Perlin temperature, moisture and eeriness fields: open meadows, green forests, autumn woods, dry
  heath and haunted barrows, each with its own trees and props, dirt cover, ground tint and structure mix. Borders
  are domain-warped so they come out ragged, and the ground tint fades across them.
- The F8 overlay shows the biome, climate values and clump density under the cursor, and the player's biome.

### Changed

- World generation is split into modules: a pure C Perlin/fBm noise library (`Noise.c`), a biome table, data-driven
  scatter features and a structure table with per-kind hooks for size, dirt, ground and props.

## [0.1.1] - 2026-10-27

### Added

- World structures stamped into the meadow, at most one per 20x20-tile region: cottages with lit windows, fenced
  graveyards, pillared crystal shrines, campfires ringed by logs and crumbling brick ruins with an archway. The local
  forest and graveyard noise decides which kind appears.
- Dev world debug overlay (F8 with `--dev`): region grid, colour-coded structure footprints and aprons, a pulsing
  lock-on to the nearest structure with an off-screen pointer and distance, and a tile inspector under the mouse.
  F9 warps to the targeted structure.

### Changed

- Terrain and scenery generation moved from `Level` into a stateless `world` module (`src/level/World.cpp`).

## [0.1.0] - 2026-10-26

### Added

- Top-down survivors-like on Vulkan2D: auto-firing Arcane Staff, Moon Blade, Warding Halo, Orbiting Knives and
  Hurled Axe, plus five passive upgrades, each up to level 5.
- Enemy waves of bats, rats, slimes, spiders, ghosts and demons, a horde ring every minute and a Cyclops boss every
  2:30 that drops a treasure chest.
- XP gems, level-up choices (keyboard, number keys or mouse), healing potions, title, pause and death screens.
- Night meadow from Kenney Tiny Town and Tiny Dungeon art with y-sorted trees, graves and will-o'-wisps.
- Light-mapped rendering with a Slang post-process (bloom, ACES tonemap, vignette, damage aberration, low-health
  pulse, grain) and hit-flash shader.
- Kenney sound effects and music loop; M mutes.
- Crash handler that writes a symbolized stack trace to `mellangan.log`.
- `dist.ps1` for versioned, 7-Zipped production builds.
- Scorpions (burst lunges), cultists (zig-zag) and fallen knights (heavy, knockback resistant), plus gold elite
  variants from minute one with 4x health, a glow and 6x XP.
- Varied night meadow: dirt clearings with proper grass rims, dense pine and autumn forests, clustered graveyards,
  logs, beehives, signposts, wells, barrels, sacks, sprouts, glowing crystals and mushrooms.
- `--dev` flag: F5 grants a level-up, F6 skips 30 seconds, F7 toggles god mode, and performance is logged every 5 s.

- Four new weapons: Storm Rod (lightning on random foes), Fire Bomb (lobbed bomb that leaves burning ground),
  Hunter's Bow (piercing arrows in your walking direction) and Reaper's Scythe (full-circle sweep).
- Three new passives: Iron Buckler (armor), Troll Blood (regeneration) and Lucky Key (critical hit chance).
- Breakable supply barrels that drop a healing potion, a gem magnet or a Starfire nova.
- Cultists throw slow, dodgeable orbs.

### Changed

- At most six weapons and six passives per run; new picks are offered only while a slot is free.

- New music: Heavy Battle 2 by MintoDog (CC0), streamed from Vorbis via stb_vorbis.
- Sound effects are peak-normalized at conversion; the axe throw was 22 dB too quiet.
- Simultaneous announcements queue instead of overwriting each other.
- Damage numbers are capped at 48 on screen; combat lights and bloom are toned down so big fights stay readable.

### Fixed

- Sound effects stopped after a while: pitch-shifted voices never drained without a flush, exhausting the voice
  pool. Voices are now flushed and the oldest one is stolen when all are busy.
- Two-tile trees and wells always draw both halves.
- White squares over heavy fights: the bloom smeared clustered damage numbers into blocks; it is now tighter and
  capped.
- Vulkan2D flushed its sprite batch for every sprite drawn in a non-default blend mode, so the additive light pass cost
  one compute dispatch and draw call per light (up to 240 a frame). Patched in the Vulkan2D submodule.
