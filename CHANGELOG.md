# Changelog

All notable changes to Mellangan are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow [Semantic Versioning](https://semver.org/).
`dist.ps1` turns the `Unreleased` section into the next version when it packages a release.

## [Unreleased]

### Added

- Five more biomes built from the sprite kit: the Jungle (jungle trees, tree ferns, bamboo, ivy, muddy ground) in hot,
  wet climates; the Savanna (straw ground with acacias, baobabs, pampas and wheat) in hot, semi-dry ones; the Ashlands
  (grey ash, charred trees, obsidian, rising embers and glowing lava pools you have to walk around); the Crystal
  Fields (lavender crystal moss, crystal spires, amethyst and ice, with drifting sparkles); and the Fungal Wood
  (purple mycelium, giant mushrooms and glowing caps, with floating spores). That makes fourteen biomes.
- Desert biome in hot, dry climates: pale dunes with saguaro and barrel cacti, dry scrub and the odd dead tree, and
  green oases with palm-ringed ponds you have to walk around. Sand and water have their own edge sets, so dunes meet
  every neighbouring biome and oasis pools meet their grass along rounded edges.
- Mire biome in the wettest woods: olive willows, reed beds, glowing purple toadstools, mud patches with their own
  edges, and fireflies drifting over the swamp.
- Blossom Grove biome in warm, mild climates: pink and white blossom trees, lilac and berry bushes, beehives and
  petal-strewn grass.
- A procedural sprite kit (`tools/sprite_kit.py`, `tools/make_sprites.py`) paints a new atlas band in the Tiny Town
  style: birch, frost birch, cypress, acacia, baobab, giant mushroom, jungle, charred, fir and snowy fir trees and a
  crystal spire; ferns, tall and dry grass, heather, thornbush, sunflowers, tulips, lavender, wheat, cattails and
  daisies; boulders (plain, mossy, snowy), sandstone, ice, obsidian and pebbles; and sand, frost, ash, bog and bloom
  tufts. `make_sprites.py --preview out.png` renders a labelled catalogue.
- A second sprite-kit band: apple, autumn birch, dead oak, weeping willow, bamboo, redwood, tree fern, blue spruce and
  Joshua trees; roses, blueberries, hydrangea, holly, ivy and sage bushes; poppies, cornflowers, bluebells,
  snowdrops and calla lilies; pumpkins, strawberries, clover, bracken, pampas, brown and glowing mushrooms; stumps,
  mossy logs, driftwood, menhirs, rock piles, stone rings, amethyst, skulls, prickly pear, agave and tumbleweed. Every
  biome draws from it, and oases grow bamboo, tree ferns and calla lilies. The kit lays out any number of two-row
  bands, and `make_sprites.py --constants` prints the matching `atlas::kt` indices.
- Biome borders grow fringe plants: each biome has a signature fringe (ferns, heather, frost tufts, bog tufts, sand
  tufts, daisies and more) that sprouts only where it meets another biome, so every pairing mixes differently.
- More varied flora everywhere: moss pines and glowing mushrooms in forests, dry bushes and dead pines on heath and in
  autumn woods, dead and spirit pines in barrows. The new plants are recoloured or drawn into the atlas by
  `tools/make_flora.py` and `tools/make_desert.py`.

### Changed

- Ground carpets: `tools/make_grounds.py` adds straw, ash, crystal moss, mycelium and lava sets (three fills and a
  grass-edge set each), and a `Carpet` biome base class lays them with grassy edges against every other biome.
- Biomes are classes: a `Biome` base holds the data and default ground, and each biome in `src/level/biome/`
  overrides only what makes it different (snowfall, petals, mud, fireflies, dunes and oases).
- Plants and ground details blend across biome borders over a few tiles, and every dirt, mud, sand and water patch
  draws with proper edges.

## [0.2.4] - 2026-10-27

### Added

- Enemies follow goals from a per-enemy queue: they look out, approach, stalk, follow a pack leader, lunge after a
  telegraphed wind-up, keep their distance to cast, and hide when looked at. Interrupt goals can jump the queue. Each
  kind has its own plan: bats flutter in, rats run in packs, slimes wait until they spot you, scorpions dart and nip,
  spiders stalk until you turn away, ghosts back off while watched, cultists hold casting range, and demons, knights
  and the cyclops wind up their strikes.
- Enemy AI runs on worker threads: thinking, moving, separation and collision are parallel passes, and only actions
  touching shared state stay on the main thread. 10,000 enemies tick in about 2 ms.

- Structures come alive near you: campfires burn and smoke, cottage chimneys smoke, and castle gates burn. Each
  structure kind can have an ambient hook beside its encounter.
- The snow biome meets other ground along a grassy snowline instead of a hard tile edge, using a grass-to-snow edge
  set generated from the dirt-edge tiles. Snowy courtyards and paths are frosted too.

- Snow biome in the coldest climate: snowy ground and frozen earth, frost-laden pines, oaks and bushes, glowing ice
  crystals, falling snow, and knights, ghosts and bats on the prowl. Its tiles are frosted copies of the Tiny Town
  ground and trees, generated into a new winter band of the atlas by `tools/make_winter.py`.
- Castles: four crenellated corner towers joined by curtain walls around a cobbled courtyard, with brick faces,
  arrow slits and a gatehouse whose portcullis or open arch is flanked by burning braziers. An open gate lets you
  into the courtyard. They stand mostly in the snow, sometimes on the heath, meadow and autumn woods. Entering one wakes
  the garrison: knights march out, and a chest waits in the courtyard, or outside a closed portcullis.

### Changed

- Cottages are proper buildings now: three to five tiles wide, one or two storeys, wood or stone walls closed by
  edge tiles, grey or red roofs with a chimney, and a gable porch over the door. The garden path leads to the door.
  Designs were proven outside the game with the new `tools/building_preview.py`.

## [0.2.3] - 2026-10-27

### Changed

- Split class into their own files and created sensible subfolders for them.

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
