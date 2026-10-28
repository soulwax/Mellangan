# Changelog

All notable changes to Mellangan are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow [Semantic Versioning](https://semver.org/).
`dist.ps1` turns the `Unreleased` section into the next version when it packages a release.

## [Unreleased]

### Changed

- Smoother terrain: Perlin noise uses sixteen evenly spread gradient directions instead of eight axis-biased ones,
  the climate is warped by a broad two-scale field instead of a fine ripple, and biomes pass through a 3x3 majority
  filter, so borders flow in long curves without one-tile spurs or pits. Dirt, carpet, sand, water and lava patches
  share one smoothing rule (a tile needs a full 2x2 block to stand, and a gap with three solid neighbours is filled),
  which removes the notches and stray tiles that made rims look rough. Ground tint now blends bilinearly across a
  four-tile lattice instead of stepping.
- Worlds are shaped differently as a result, so saved map charts from earlier versions are discarded once.

### Fixed

- Enemies no longer stop dropping gems once 450 lie uncollected on the map: the cap that silently merged new drops
  into a gem far away is gone. Uncollected gems more than 300 tiles from the player are cleared instead.

## [0.4.1] - 2026-10-28

### Changed

- The class screen's info card is rebuilt: larger text that wraps cleanly, the same clear order for every class
  (weapon, trait, skill with its key, arsenal with talents marked, nature, favoured upgrades), and a style of its own
  per class: a pixel-art sigil with highlights, shading and twinkling accents (Mage star, Ranger arrow, Knight shield, Alchemist flask with rising bubbles,
  Stormcaller bolt, Reaper crescent, Priestess sun, Duelist crossed blades) heads the card, patterns its banner,
  marks the trait and guards its corners, all in the class colour.

## [0.4.0] - 2026-10-28

### Added

- Classes: a class screen after the title shows eight heroes on pedestals, the chosen one lit and raised, with its
  signature weapon, trait, skill and strengths beside it. The Mage (+15% experience), Ranger (dodges 15% of blows), Knight
  (a bulwark halves one blow every 6 seconds), Alchemist (fallen foes may leave potions), Stormcaller (every 20th kill
  calls lightning) and Reaper (every kill mends health) each have their own sprite and stats, and unlock through the
  old starting-weapon milestones. Locked classes stand in shadow with their requirement.
- Class kits: every class has a signature skill on Space with a cooldown shown on the HUD (Mage Blink, Ranger
  Volley, Knight Rally, Alchemist Elixir, Stormcaller Thunderclap, Reaper Death's Embrace), two class-only upgrades
  in its level-ups (Frost Nova and Arcane Mind; Piercing Shot and Fleet Foot; Judgement and Oathbound; Acid Flask and
  Distillation; Chain Lightning and Conduit; Wraith Bolts and Bloodlust), and three favoured staples it is offered
  more often. Class upgrades and the class screen wear the class colour.
- Two heroines join the Alchemist: the Priestess (shrines kindle 30% faster; Sanctuary heals and drives foes back;
  Radiance and Grace; unlocked by kindling 10 shrines) and the Duelist (critical hits strike far harder; Dance of
  Blades dashes and cuts both ways; Whirling Edge and Keen Eye; unlocked by surviving 10 minutes). The codex now
  counts shrines kindled.
- The class screen comes alive: a near-opaque backdrop with a vignette, a compact panel with the class's strengths
  and weaknesses as tinted chips, and the chosen hero in an attack stance (wind-up, lunge, weapon swing) inside a
  pulsing aura, with spinning rune circles, rising motes and a spell burst at the strike.

### Changed

- Each class is a `Hero` subclass in `src/hero/`, overriding only the hooks that make it different (equip, tick,
  guard, wisdom, slay). The starting-weapon row on the title panel is gone.

- Deeper class kits: each class gains a second ability and a talent that hooks into its play. Mage: Meteor and
  Echoing Blink (Blink detonates and recharges faster). Ranger: Scattershot and Second Wind (kills may reset Volley).
  Knight: Crusader Strike and Retribution (blows against you strike back). Alchemist: Transmute (foes turned to gold
  drop rich gems) and Volatile Mix (fallen foes may explode). Stormcaller: Static Field and Overcharge (Thunderclap
  calls more bolts). Reaper: Soul Siphon and Harvest (kills shorten Death's Embrace). Priestess: Aegis and Beacon
  (Sanctuary heals and burns more). Duelist: Flurry and Riposte (parry and cut back). Upgrades can now react to
  kills, incoming blows and the class skill.
- Smarter monsters: they lead their chase towards where you are heading, fan out instead of trailing in one line,
  cultists aim their orbs ahead of you, a wounded foe wakes its neighbours, and most foes break off to regroup once
  when badly hurt.

### Fixed

- Passive upgrades (Heartroot, Ogre Hammer, Hourglass Tonic and the rest, and the class passives) could apply no
  bonus at all, or scramble a stat, in Windows builds: MSVC laid out their stat pointer differently depending on
  whether the player type was complete, so two parts of the game disagreed about where the bonus was stored.


## [0.3.1] - 2026-10-28

### Added

- New-best moments: a NEW BEST banner when you outlast your record on a world, and the death screen marks a record and
  tells you how far away dawn was. Classic runs now count towards the past-worlds list too.
- Daily world: a fourth world mode gives everyone the same land for the day.
- Codex (C on the title or pause screen): regions walked, relics found, a bestiary of kills, places entered, and totals
  for runs, dawns, best time and best streak, kept between launches.
- Weapon evolutions: a maxed weapon whose partner passive you own evolves at the next chest into a stronger form
  (double damage, faster), such as the Archmage's Staff, Eclipse Blade or Thunder Sceptre.
- Bounties: every world rolls three goals (slay a foe type, kindle a shrine in a biome, find a place, walk regions,
  survive, keep a kill streak), listed on the map. Each pays out a chest, and the same world always has the same ones.
- Kill streaks: kills within 2.5 seconds of each other chain into a streak worth up to +50% XP, shown on the HUD.
- Biome flavours: each biome nudges play, shown above the minimap. Mires slow your step, snow slows everyone,
  crystal fields and barrows give more XP, fungal spores and blossom groves heal, and ash embers scorch nearby foes.
- Night omens: about one night in five is a Blood Moon (more elites, +50% XP), an Aurora (shrines kindle twice as
  fast) or a Gale (everyone is swifter), announced at the start.
- Unlockable starts: begin with the Hunter's Bow, Moon Blade, Fire Bomb, Storm Rod or Reaper's Scythe once you have
  earned them (see a dawn, slay 1000 foes, find 3 relics, walk 10 regions, a 100 kill streak). Pick with Left/Right
  on the title.

## [0.3.0] - 2026-10-28

### Added

- Relics: six unique treasures far stronger than ordinary upgrades. Every chest offers one as a gold fourth card,
  and any level-up has a 3% chance to. The Sunfire Crown flares around you, the Tempest Eye calls three lightning
  strikes a second, the Phoenix Feather raises you once from death, the Heart of the Grove adds health and
  regeneration, the Frozen Hourglass speeds every weapon and your step, and the Obsidian Signet adds damage and armour.
  Each can be taken once per run, takes no upgrade slot, and is framed in gold on the HUD.
- The world map remembers: land charted in earlier runs shows faded on the minimap and the full map, along with the
  structures sighted there.
- The death and dawn screens show who felled you and how much of the damage each weapon dealt.
- World seeds: a panel on the title screen picks the Classic world, a Random one each run, or a Custom code typed
  in (letters and digits, up to 12). The land behind the title regenerates as you choose. The code shows on the map,
  pause, death and dawn screens so worlds can be shared, and every world keeps its own chart.
- Past worlds: each finished run records its world code with your best time, run count and whether you saw dawn.
  The six most recent are listed on the title panel; pick one with Up/Down or a click to play the same land again.
  The choice is remembered between launches.

## [0.2.6] - 2026-10-27

### Added

- A world map that charts the land as you walk it: a minimap in the corner names the region you stand in, and Tab
  opens the full map with panning, three zoom levels, every structure you have sighted (shrines you lit in gold, the
  next shrine pulsing) and a legend of the regions charted so far. Entering a new biome for the first time announces
  it, and the dawn and death screens count the regions you walked.

### Fixed

- Text and particles drew from the wrong rows of the font and particle sheets: their row lookups had become float
  divisions and lost their rounding down.
- Windows builds failed after the include sort put `dbghelp.h` ahead of `windows.h`.

## [0.2.5] - 2026-10-27

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
