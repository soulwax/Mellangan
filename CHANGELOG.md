# Changelog

All notable changes to Mellangan are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow [Semantic Versioning](https://semver.org/).
`dist.ps1` turns the `Unreleased` section into the next version when it packages a release.

## [Unreleased]

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

### Changed

- New music: Heavy Battle 2 by MintoDog (CC0), streamed from Vorbis via stb_vorbis.
- Sound effects are peak-normalized at conversion; the axe throw was 22 dB too quiet.
- Simultaneous announcements queue instead of overwriting each other.
- Damage numbers are capped at 48 on screen; combat lights and bloom are toned down so big fights stay readable.

### Fixed

- Sound effects stopped after a while: pitch-shifted voices never drained without a flush, exhausting the voice
  pool. Voices are now flushed and the oldest one is stolen when all are busy.
- Two-tile trees and wells always draw both halves.
- White squares over heavy fights: Vulkan2D re-flushed its sprite batch for every additive sprite, which corrupted
  lights under load (patched in the Vulkan2D submodule).
