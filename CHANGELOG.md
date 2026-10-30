# Changelog

All notable changes to Mellangan are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow [Semantic Versioning](https://semver.org/).
`dist.ps1` turns the `Unreleased` section into the next version when it packages a release.

## [Unreleased]

## [0.5.10] - 2026-10-30

### Added

- Every region now has a world event, not just the ten newer ones. Each one is announced, telegraphed and lasts a few seconds:
  - A Warm Breeze heals in the Meadows.
  - The Old Oaks Shelter in the Greenwood, pushing foes out of their circle.
  - A Harvest Gale sweeps through the Autumn Woods.
  - Wildfire spreads across the Heath.
  - The Barrow opens and skeletons climb out.
  - A Blizzard howls in the Snowfields.
  - Petals fall in the Blossom Grove, cleansing and healing.
  - Bog gas rises in the Mire.
  - A sandstorm sweeps the Desert.
  - Vines grasp in the Jungle.
  - A stampede thunders across the Savanna.
  - The ground erupts in the Ashlands.
  - The crystals resonate in Crystal Fields, charging your class skill.
  - A spore bloom in the Fungal Wood heals you and poisons foes.
  - Rocks fall in the Badlands.
  - Wolves encircle you in the Taiga.

  Events start after the first 40 seconds, and the older regions have them less often than the newer ones.
- Every region has its own ambient life:
  - butterflies in the Meadows;
  - falling leaves in the Greenwood;
  - maple leaves in the Autumn Woods;
  - drifting seed fluff on the Heath;
  - barrow mist;
  - snowflakes;
  - cherry petals;
  - bog mist;
  - blowing sand;
  - falling jungle leaves;
  - savanna gusts;
  - falling ash;
  - crystal glints;
  - spores;
  - badlands dust;
  - salt glare;
  - rising bubbles on the Floodplain;
  - moonlit motes;
  - rising bubbles in the Sunken Ruins;
  - fen embers;
  - amber leaves;
  - shooting stars over the Starfall Basin.
- Real weather. Fronts roll in and out every minute or two, and what you get depends on the region:
  - **Clouds** cast drifting shadows across the ground.
  - **Rain** slants with the wind, ripples where it lands and darkens the scene.
  - **Storms** bring heavier rain, a darker sky and lightning that flashes the screen and strikes nearby foes.
  - **Snow** in cold lands settles briefly on the ground.
  - **Fog** banks roll through mires, barrows, fungal woods and caverns.
  - **Dust** storms streak across deserts and badlands.
  - **Ash** falls over the Ashlands and the Ember Fen.

  - **Rain leaves puddles** that spread while it pours and dry slowly afterwards. Drops fall in depth, near ones longer and brighter than far ones, and each lands with a splash crown before its ripple.
  - **Snow settles**: the ground whitens tile by tile as a snowfall goes on, then thaws.
  - **Ash and dust** leave a light scatter on the ground.
  - **Lightning** flickers twice, and sheet lightning lights the clouds between strikes.

  The current weather (drizzle, rain, thunderstorm, snowfall, fog, sandstorm or ashfall) is named above the region name by the minimap. Fog takes on each region's light: green in the Mire, violet in the Hollow Caverns.

  Weather is no longer an even sheet:
  - Rain and snow fall in showers and bands that drift with the wind, and clouds gather darker over them.
  - Each new front brings a wind from its own direction, which slowly wanders. Gusts sweep across the ground, so drops caught in one slant harder.
  - Snowflakes flutter on their own rhythm, and a few big flakes drift close by.
  - Clouds and fog banks come in irregular clusters of varied shape and speed.
  - Puddles and dust gather in low ground and spread outward as the weather goes on.
  - Snow settles in patches with soft pixel edges that slowly close into a white blanket over about a minute, and thaws the same way.

  The leaves, petals and seeds drift with the same wind.
  - Snow settles on trees and shrubs as well as the ground, so a forest whitens with the snowfall.
  - Butterflies, petals and seed fluff shelter from rain, snow and dust, and come back when it clears.
  - Rain soaks the ground, which darkens as it gets wetter. Puddles catch your torchlight, and lightning makes them flash.
  - Rain ripples only where it lands in water; on grass it just splashes.
  - Strong, lasting rain floods low ground into pools that fill tile by tile with proper water edges. Rain rings on their surface, and the pools mirror whatever stands in them: trees, foes and you. Storms flood faster, and pools drain slowly once the rain stops.
  - Trees bend in the wind: their crowns lean a pixel or two in gusts, more in a storm.
  - You leave footprints in snow that stay until the snow melts, and fading muddy prints on wet ground.
  - Thunder follows the flash: a near strike cracks almost at once, a far one rumbles later and lower, and distant sheet lightning brings a low roll.
  - Rain softens the distance with a light haze.
  - Rain and wind can now be heard: a soft rain bed that swells under showers and storms, and a low wind that rises with gales and sandstorms. Both stay well under the music and the fight. Each region has its own odds of each front: the Storm Steppe is almost always stormy, the Desert is mostly clear with the odd sandstorm, and the Hollow Caverns are usually fogged in. The cloud, fog and splash sprites are pixelated from the NS1 weather VFX packs.
- Each region has its own light: a warm glare in the Desert, a blue hush in the Moonlit Grove, a dim dusk in the Hollow Caverns and a red cast in the Ashlands. The light shifts gradually as you cross a border.
- Seven new particle icons: maple leaf, raindrop, gust, seed fluff, falling star, butterfly and spore.
- Dev: `--biome N` starts a `--dev` run inside biome N, F10 cycles the weather, and `--biome-map <file.ppm>` writes a map of the world's biomes.

### Changed

- Region events are defined in one table, which makes each event easy to tune.
- The world is calmer. Biomes now form large, coherent regions about four screens across, with organic borders, instead of a patchwork of thin strips and stray patches. The ten newer biomes form their own wild lands rather than ribbons running through everything else, and the dark lands (Ashlands, Crystal Fields, Fungal Wood and Barrows) no longer have green meadow holes inside them. The ten newer biomes also no longer scatter grass-rimmed dirt patches across their ground. The map has about 92% fewer biome borders, and the start area stays gentle. World layouts differ from earlier versions.

## [0.5.9] - 2026-10-30

### Added

- Two new bosses join the Cyclops in rotation: the Pack Lord, a giant Alpha Wolf that brings a pack of six to eight and howls them into a frenzy, and the Lich, a giant Necromancer that raises four skeletons at a time. Bosses wear a pulsing crimson ring, shrug off most knockback, survive a Nova and drop a chest.
- A boss health bar at the bottom of the screen names the boss you face and counts any others still alive.
- The Pack Lord's Blood Moon Howl flashes a crimson sunburst and calls three more wolves while its pack is under ten.
- The Lich's Death Ring: each cast also fires a slower ring of ten orbs.
- A low-health warning: below 30% health the screen edges pulse red like a heartbeat, faster as health drops.

### Changed

- Heavy moments land with a brief hit stop: taking a hit, killing an elite and felling a boss. It is rate-limited so a crowd never stutters the game.
- Levelling up has a beat: a gold sunburst and glints burst from you, with a moment of safety, before the choices appear.
- Gems and pickups spiral in toward you instead of sliding in a straight line.
- Twelve monsters no longer die in generic gore:
  - Skeletons collapse into bones and dust.
  - Icebacks shatter into ice.
  - Sporelings and Mycelial Brutes burst into spore clouds.
  - Burrowers crumble into rocks and dirt.
  - Knights shed armour plates and sparks.
  - Cultists and Necromancers release violet smoke and freed souls.
  - Thieves scatter coins.
  - Mire Leeches leave a puff of bog.

## [0.5.8] - 2026-10-30

### Added

- Five difficulties, in homage to DOOM: I'M TOO YOUNG TO DIE, HEY, NOT TOO ROUGH, HURT ME PLENTY, ULTRA-VIOLENCE and NIGHTMARE! Pick one on the title screen with Left/Right or a click; the choice is remembered.
  - HURT ME PLENTY is the new default and noticeably gentler than before: slower spawn growth, smaller hordes, fewer elites, sturdier player and faster levelling.
  - ULTRA-VIOLENCE is the previous balance, unchanged.
  - NIGHTMARE! has faster foes, bigger hordes, more frequent bosses and many more elites.
- Game balance lives in `res/balance.ini`: spawn pacing, hordes, bosses, foe health, damage and speed growth, elites, player health, damage taken, invulnerability and the experience curve, with shared values and per-difficulty overrides. A `balance.ini` in the save folder overrides it for fine-tuning; changes apply from the next run.

## [0.5.7] - 2026-10-30

### Changed

- Each blade has its own trail. The Moon Blade sheds silver crescents and glints (violet once evolved), Crusader Strike is a gold double cleave that throws sparks, pebbles and dust, and Flurry and Dance of Blades scatter pink petals.
- Whirling Edge and Riposte spin a ring of ghostly swords trailing petals instead of the Reaper's scythe; the Reaper's Scythe and Death's Embrace leave crimson ghost blades in dark smoke, and the Knight's Rally sweeps in gold sparks.
- Hunter's Bow arrows trail a green wind streak and shed leaves, Piercing Shots are heavy steel arrows that ring out at every foe they pass through, and Scattershot fans out a burst of feathers.
- Hurled Axes leave a spinning afterimage and knock out chips, Chakrams whirl inside a blue ring trailing sparks, Glaive thrusts streak the air, Orbiting Knives leave a short motion blur, and the Censer releases drifting puffs of incense.
- Melee slashes are a crisp pixel crescent instead of a soft gradient.
- The eight newer weapons each have their own look:
  - Ricochet Sling stones kick up dust and pebbles at every bounce.
  - Harpoons trail a rope and splash droplets.
  - Ground Sigil is a turning rune circle that ripples frost on every pulse.
  - Boomerangs leave an afterimage and gusts of wind.
  - Mortar shells drop onto a gold landing ring with a smoke trail and blast a smoking crater.
  - Thorn Lash is a thorny vine that sheds leaves.
  - Prism Beam is a shimmering three-colour ray that sparkles at its end.
  - Spirit Swarm sends trailing teal wisps that mark what they hit.
- Every class skill has a signature moment:
  - Blink leaves a violet afterimage, a trail of stars and an arcane sigil where you land.
  - Volley fans out leaves with a burst of wind.
  - Rally wraps you in a golden shield for exactly as long as you are untouchable, over a sunburst.
  - Elixir bubbles up green.
  - Thunderclap marks the ground beneath you with a storm sigil.
  - Death's Embrace pulls crimson souls out of every foe it catches, in a burst of dark smoke.
  - Sanctuary is a gold sunburst that lifts holy glints.
  - Dance of Blades and Broadside leave an afterimage and a trail of petals or sea spray along the dash.
  - Rootbound lashes green vines to every foe it slows and throws leaves.
  - Sentry lands with a ring and flying chips.
  - Revelation sweeps a violet wave and stars the foes it marks.

### Added

- Developer flag `--hero N` (with `--dev`) starts runs as class N (0 Mage … 11 Corsair) without changing the saved class choice.

### Fixed

- Savanna, Ashlands, Crystal Fields, Fungal Wood, Badlands and Taiga have their own ground again (straw, ash, crystal moss, mycelium, lava, red clay and needles). Since 0.5.0 they drew a dark empty floor, because the standalone expansion art tool dropped the ground sets from the atlas. The tool now runs the full pipeline, and the atlas check rejects empty bands.
- Effects that carry information (lightning arcs, rings, ground marks, healing motes) no longer disappear at random with reduced particles turned on.

## [0.5.6] - 2026-10-29

### Added

- The Alpha Wolf howls, the Burrower erupts with a crack of stone and the Necromancer raises the dead with a rattle of bones, each with its own sound. These voices play only on screen and are rate-limited, so a large pack still sounds like one cue.
- Developer flag `--arm "Name,Other!"` (with `--dev`) starts every run with those upgrades at full level; `!` evolves one, and a leading `=` drops the class starter and skips level-ups, so one ability can be watched alone.

### Changed

- Frost Nova looks like one: a bright ring of frost bursts outward, flinging ice shards and leaving glinting frost on the ground.
- Meteor drops a molten rock from the sky instead of lobbing a bomb: a gold ring and a growing shadow mark where it lands, a fiery streak follows it down, and the impact throws rocks and leaves a burning, scorched crater.
- The Arcane Staff fires violet arcane missiles that spin, trail sparks and leave a fading rune where they strike; the Archmage's Staff fires larger ones.
- Chain Lightning arcs from you through each foe in turn instead of calling bolts from the sky, and Static Field snaps arcs to every foe it shocks inside a crackling edge ring.
- Storm Rod bolts fork, flash a ring across the ground and leave a scorch mark.
- Judgement is a pillar of holy light: a gold rune circle tightens on the target, the beam slams down and a halo ring lingers.
- Warding Halo shows its reach as a ring of slowly turning runes that flares with each pulse.
- Radiance bursts as a gold sunburst, and a glint flies back to you from every foe it mends you for; Aegis wraps you in a golden shield for exactly as long as you are untouchable.
- Soul Siphon pulls crimson souls out of its victims and into you, and Wraith Bolts are bone-white spirits trailing dark smoke.
- Fire Bombs fly with a sputtering fuse, burst with a ring and a scorch mark, and leave pools of dancing flame.
- Acid Flasks tumble through the air, shatter into glass and leave sizzling green pools of popping bubbles instead of fire.
- Transmute turns the foes it kills into gold statues that flash, crumble and scatter coins.
- Glows, rings and shadows are pixel art now: rings are crisp one-pixel outlines drawn at their true radius, glows are banded and dithered, and shadows are hard-edged; lighting stays smooth.

- Each of the ten expansion biomes now favours structures that suit it (mines in the Hollow Caverns, ruins in the Sunken Ruins, standing stones in Starfall Basin) instead of one picked by position.
- Classic castles, graveyards and mines always keep their own encounter; biome challenges only take over smaller sites there.
- Banners (region names, encounters, hordes, bosses) sit on a dark plate framed by thin gold rules, so they stay readable over a dense crowd.
- Frost, spore and ember trails show a red warning only while they arm, then settle into a soft ring in their element's colour, so crowded fights stay readable.

### Fixed

- Moonlight and storm events no longer tell you to move to safety, count as a handicap or show up as your cause of death; they now invite you to step inside.
- Removed a Cyclops entry from the Badlands spawn list that could never trigger.
- The F3 frame counter sits above the class-skill panel instead of overlapping it.

## [0.5.5] - 2026-10-29

### Changed

- The view now fills the window on every platform: 16:9 windows show the familiar 480×270 world, wider or taller windows reveal more of it (up to 640×270 or 480×360) instead of showing black bars, and the world scales by any factor with crisp pixel-art filtering. Enemy spawn rings move out with the view so enemies still appear off screen.

### Added

- Native Apple Silicon macOS builds as a self-contained `Mellangan.app` (macOS 12+) with embedded MoltenVK, an icon and bundle metadata; `setup-macos.sh` installs a reproducible Homebrew toolchain plus pinned `slangc` and MoltenVK.
- macOS windows render at full Retina resolution, and startup logs the renderer, device and window pixel size.
- `dist.sh` packages the macOS build as an ad hoc signed `Mellangan.app` zip instead of a bare executable with a launcher script.
- On macOS the log and Vulkan2D error file are written to the application support folder instead of next to the executable.

### Fixed

- Vulkan2D enables portability enumeration on macOS only when the instance offers it (SDL always requests it), checks descriptor-indexing features by name, clamps the texture limit to the device, drops obsolete device layers, and keeps running when the validation layer is missing or fails to load.

## [0.5.4] - 2026-10-29

### Changed

- Improved hostile attack warnings and status indicators, and added a persistent reduced-particles option.
- Split biome challenge rewards, guards, and objective rules into reusable modules.
- Narrowed biome ground, colour, and vegetation cisitions by half for sharper boundaries.

### Added

- Biome challenges now feature local enemies and themed permanent run rewards, with stronger rewards in exotic regions.

### Fixed

- Regional monsters stay in their regions. Wandering spawns and the minute horde draw only from the classic roster
  again, instead of filling every biome with whichever regional monster unlocked last, such as cinder golems and
  mycelial brutes after three minutes. Wolves, burrowers, necromancers and the ten expansion monsters now appear only
  where their biome calls for them.
- The ten expansion biomes host fitting monsters instead of an index-shifted pair. For example, the Ember Fen has
  ember imps and leeches rather than frost wisps and icebacks, and the Glass Desert has glass mantises and burrowers.

## [0.5.3] - 2026-10-29

### Changed

- Favor familiar biomes around spawn and introduce exotic regions gradually farther from the starting point; refresh older explored maps for the new geography.
- Refactored reusable GUI panels, cards, buttons, badges, and choice navigation into shared components.
- Made enemy attack circles red with warning ticks and added distinct particle deaths for spirits, demons, slimes, insects, flyers, and organic enemies.

## [0.5.2] - 2026-10-29

### Changed

- Replaced decorative combat rings with drifting particles for halos, sigils, fire, incense, and lightning strikes while retaining attack warnings.
- Automatically apply a level-up when only one choice is available.
- Increased XP requirements exponentially after the opening levels, making very high levels a long-term goal even during sustained kill streaks.

## [0.5.1] - 2026-10-29

### Changed

- Enlarged biome geography by 2.5 times and blended ground materials, colours, and vegetation across wider borders while preserving paths, structures, lava, and oasis water.
- Migrated older explored maps to the new geography without clearing explored areas or Codex progress.

### Fixed

- Removed an invalid free of SDL3's cached base path that could cause a Debug heap assertion during shutdown.

### Added

- Deterministic biome-spacing, blend-continuity, cache, reachability, and chart-migration tests, plus map previews and an isolated in-game boundary viewer.

## [0.5.0] - 2026-10-29

### Added

- Comprehensive end-game run statistics with a 21-tier performance ranking (F- to S+), distance ran tracking with kilometre overflow, total damage dealt, and signature class styling.
- Overhauled menu aesthetics with un-squashed level-up cards, psychedelic background illumination shader, and custom multi-color symbol sprites for shrine offerings.

## [0.4.8] - 2026-10-29

### Fixed

- Refined level-up card proportions and typography layout:
  - Relaxed card dimensions and line spacing to resolve text squashing between titles and descriptions.
  - Right-anchored tier progression tags and hero class labels beside shortcut badges for clean visual breathing room.
  - Enhanced procedural category symbol badges with proportional spacing.

## [0.4.7] - 2026-10-29

### Added

- Centered lean level-up cards with procedural category icons and tier-colored borders:
  - Procedural 11x11 symbol matrices for weapons, passives, relics, evolutions, class upgrades, and vitality hearts.
  - Progression tier border styling (orange relic, gray tier 1, white tier 2, green tier 3, blue tier 4, purple tier 5+).
  - Shimmering animated silver filigree perimeter with traveling glints for hero class upgrades.
  - Post-processing pause shader effect with frosted slate-blue desaturation and radial vignette.

## [0.4.6] - 2026-10-28

### Added

- Elevated handcrafted 16x16 pixel-art character sprites and class sigils:
  - Four expansion heroes (Warden, Tinkerer, Oracle, Corsair) with distinctive silhouettes, 3-tone lighting, and custom class sigils.
  - Nine expansion monsters (Bog Lantern, Glass Mantis, Dust Vulture, Frost Wisp, Iceback, Sporeling, Mycelial Brute, Ember Imp, Cinder Golem) with cohesive contours and material highlights.
  - Updated HUD skill display to render each hero's unique sprite.

### Fixed

- Resolved false-positive antivirus heuristic triggers by omitting debug stream tables from Release builds, disabling Windows unhandled exception filters, and embedding application UAC manifest.


## [0.4.5] - 2026-10-28

### Added

- 10 new monsters with telegraphed combat behaviors and status interactions:
  - **Mire Leech** (Mire & Wetlands): latches on at close range, draining health until broken off.
  - **Bog Lantern** (Mire & Floodplains): lures with ambient glow, then bursts into an expanding marked ring.
  - **Glass Mantis** (Desert & Glass Desert): lines up a swift telegraphed dash across the sands.
  - **Dust Vulture** (Badlands & Desert): circles overhead out of reach before diving in for a strike.
  - **Frost Wisp** (Snow & Taiga): leaves a trail of slowing frost patches on the ground.
  - **Iceback** (Snow & Taiga): winds up a heavy charging ram that chills on contact.
  - **Sporeling** (Fungal Wood & Overgrowth): releases a lingering poisonous spore cloud on approach.
  - **Mycelial Brute** (Fungal Wood): pulses a cleansing heal that purges burn and poison from nearby allies.
  - **Ember Imp** (Ashlands & Ember Fen): darts rapidly, leaving scorch patches in its wake.
  - **Cinder Golem** (Ashlands): erupts into an expansive fiery hazard upon destruction.
- 10 new mechanically distinct biomes reaching a total of 26:
  - **Salt Flats**, **Floodplain**, **Moonlit Grove**, **Sunken Ruins**, **Ember Fen**, **Amber Woods**, **Storm Steppe**, **Glass Desert**, **Hollow Caverns**, and **Starfall Basin**.
  - 30 new characteristic props, regional ground carpets, and dedicated structure weighting per biome.
- Four new character classes reaching a total of 12:
  - **Warden**, **Tinkerer**, **Oracle**, and **Corsair**, each with unique starting equipment, active skills, passive traits, and unlock requirements.
- Eight new weapons reaching a total of 36:
  - **Ricochet Sling**, **Harpoon**, **Ground Sigil**, **Boomerang**, **Mortar**, **Thorn Lash**, **Prism Beam**, and **Spirit Swarm**, each with five upgrade tiers.
- Eight new run-changing relics reaching a total of 14:
  - **Carnage Bell**, **Last Bastion**, **Pilgrim Compass**, **Plague Seed**, **Recall Mirror**, **Weather Vane**, **Hunter's Writ**, and **Mercy Locket**.
- Interactive shrine offerings: kindling now offers meaningful choices between run blessings and voluntary sacrifices.
- Dynamic biome events and risk/reward encounter structures (ambush caches, guarded altars, roaming caravans, corrupted springs).
- Reserved and validated fixed 16 x 16 atlas bands for expansion actors, props, ground sets, and UI icons.
- Higher-fidelity pixel-art painter primitives with deterministic outline, cluster, shadow, and highlight tools (`tools/sprite_kit.py`).
- Deterministic asset-pipeline verification script (`tools/make_expansion.py`, `tools/atlas_layout.py`).
- Headless expansion test suite verifying reachability of all 26 biomes, 26 monsters, 36 weapons, 14 relics, 12 classes, shrine choices, and Codex persistence (`--test`).

### Gameplay

- Added shared burn, chill, poison, and marked status effects with duration refresh, potency caps, immunity flags, weapon damage attribution, and cleanup on death or revival.
- Frost Nova now slows enemies by 35% for two seconds; fire pools leave a short burn after enemies move away.

## [0.4.4] - 2026-10-28

### Added

- Four signature mobs tied to exploration and biome identity:
  - **Burrower** (Badlands & Desert): travels underground as a churning earth mound and erupts beneath the player after
    an expanding telegraph ring.
  - **Wolf & Alpha Wolf** (Taiga & Snow): wolves flank from the sides in coordinated packs while the alpha's howl speeds
    them up.
  - **Necromancer** (Barrows & Graveyards): retreats to maintain distance while raising skeletons from graves.
  - **Treasure Thief** (Rare, Worldwide): flees on sight and attempts to escape through a portal after ten seconds;
    defeating it yields an immediate treasure chest.
- Three new weapons with unique combat roles:
  - **Glaive**: a directional polearm thrust that impales and knocks back lines of encroaching enemies.
  - **Chakram**: a returning bladed disc that carves through crowds on its forward path and returns to the wielder.
  - **Censer**: swings smoldering clouds of aromatic incense that linger and scorch enemies caught in the fumes.
- New bounty contracts to hunt the Taiga alpha and intercept fleeing treasure thieves.
- Codex tracking and bestiary entries for all new monsters.
- Headless memory leak, concurrency, and level lifecycle test suite (`--test` CLI flag).

### Fixed

- Fixed heap memory leak of the base path string returned by `SDL_GetBasePath()`.
- Fixed texture resource leaks on partial initialization failure in `Screen` and added automatic RAII cleanup.
- Ensured WAV audio samples and music stream buffers are always freed during audio shutdown even if the audio device
  fails or closes early.
- Fixed pool allocator memory retention: 64 KB slabs are now tracked and fully reclaimed via `pool_shutdown()` on exit.
- Eliminated multithreaded data races during worker enemy thinking: synchronized pool allocations, level entity
  additions, spawner rolls, and retirement queues.
- Replaced non-atomic particle counters with `std::atomic` and made PRNG state thread-local, preventing dropped counts
  and memory desync during parallel AI execution.

## [0.4.3] - 2026-10-28

### Added

- Badlands: red clay plains with banded mesas, hoodoos, twisted junipers, ocotillo and bleached ribs, where dry, warm
  land runs mundane. Scorpions, rats and cyclopes roam it, and it pays +20% experience.
- Taiga: cold, wet spruce forest on needle litter with golden larch stands, lichen rocks, shelf-fungus stumps,
  lingonberries and cotton grass. Its deep needles slow everyone a little.
- Standing stones: a ring of rune stones around a glowing altar. Entering one restores you to full health, and the
  altar keeps humming with rising sparks.
- Abandoned mines: a timber-framed shaft in a rock face with rails, an ore cart, ore piles and a lantern. Entering
  wakes spiders and bats from the dark and leaves a chest by the tracks. Mines favour the badlands and deserts.
- `tools/make_frontier.py` paints the new art into a free atlas region; `make_grounds.py` adds red clay and needle
  ground sets.

### Fixed

- The codex reads each saved row on its own, so adding places no longer shifts the shrine count into them.

### Changed

- Biome shares shift to make room (meadow, greenwood, desert and snowfields give the most), so saved map charts from
  earlier versions are discarded once.

## [0.4.2] - 2026-10-28

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
