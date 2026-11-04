# Changelog

All notable changes to Mellangan are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow [Semantic Versioning](https://semver.org/).
`dist.ps1` turns the `Unreleased` section into the next version when it packages a release.

## [Unreleased]

### Added

- Smooth motion on high-refresh displays: the world still steps 60 times a second, but every sprite, light and the camera are drawn eased between the last two steps, so 120 and 144 Hz screens no longer show bunched, uneven movement. Anything that jumps more than three tiles in one step, such as a blink or a respawn, snaps instead of sliding. At 60 frames a second or less nothing is blended, so 60 Hz screens gain no delay and no cost.
- Continuous integration on GitHub Actions: every push to `main` and every pull request builds with MSVC at `/W4 /WX` (Debug and Release) and with Clang at `-Werror` under AddressSanitizer and UBSan on Linux and on Apple Silicon macOS, and runs `--test` on all three.
- `--bench` reports the 99th-percentile and worst frame cost next to the averages, so hitches show up instead of hiding in the mean.
- A frame-time graph under the F3 counter: the last 120 frames as bars, tick time in blue under render time, gold above half a frame and red above a full 60 Hz frame, with a line at 16.7 ms and the worst frame in view.
- `--bench-gate` runs the bench up to 10,000 enemies and exits with code 2 unless that row holds 55 fps with a frame p99 under 16.7 ms. `dist.ps1` runs it on the Release build before packaging and restores VERSION and CHANGELOG.md if it fails; `-SkipBench` packages without it.

### Changed

- `--test` is fully repeatable: it seeds its random numbers, and crowd pushing alternates by entity id instead of memory address. A rare bastion-defence test failure came from those two sources.
- `--test`, `--biome-map`, `--loot-sim` and `--vendor-sim` run before the window and renderer start, so they work on machines without a GPU.
- The Windows application manifest is embedded as a CMake source instead of a raw `/MANIFESTINPUT` link flag, so Ninja and clang-cl builds link too; the embedded manifest is byte-identical.
- stb's headers are included as system headers, so Apple's `sprintf` deprecation inside `stb_image_write.h` no longer fails `-Werror` builds on macOS.

- Big crowds cost about a fifth less CPU. At 10,000 foes, Release ticks in 2.5 ms instead of 3.1 and renders in 2.6 ms instead of 3.6, and the frame p99 drops from 11.0 to 7.4 ms:
  - foes come from the entity pool again: they had outgrown its 512-byte limit, so every foe had become a separate heap allocation. The pool now serves objects up to 1,520 bytes, and the build fails if any foe class outgrows it;
  - status effects keep a bitmask of what is running, so the common foe with no effects skips its effect scan every tick;
  - the crowd grid is built in one pass over the foes instead of two;
  - the draw queue takes on-screen foes from the crowd grid instead of testing every foe in the world.
- Gear stats (`ItemStats`, 48 values) live outside the creature and are allocated only for creatures that wear gear, so a foe shrinks from 1,000 to 832 bytes. Reads go through `bonus()`, which returns a shared all-zero block for foes without gear, and writes through `kit()`.
- A creature's status effects are private to `Status`: code reads them with `effect(kind)` and cuts one short with `end(kind)`, so the bitmask of running effects is always exact. `power()` checks that bit first, so a foe with no effects no longer reads its effect slots for its speed, damage and fear each tick.
- `Enemy::die` (124 lines) reads as a list of steps: retire, `tally`, `onKill`, `spoils`, `bossFalls`, `championFalls`, then `gilt` or `remains`. The chains of enemy kinds that picked a corpse style and blood colour are now two `switch` tables, `remainsOf` and `redBlooded`. Random draws happen in the same order, so runs replay exactly as before.
- `Game::tick` (109 lines) is split into `hotkeys`, `route` (which key opens which menu, with `nearby` choosing the shop, stash or bench you stand at) and `play`. The campaign's building, gathering, autosave and skill-point steps moved to `GameCampaign.cpp` as `place`, `gather`, `keep` and `bankLevels`.
- The content check at startup also validates unique items, so a unique naming a missing item base or upgrade is reported instead of silently never dropping.
- `--bench` opts its process out of Windows power throttling, so a bench run in an unfocused window is no longer slowed to efficiency clocks.

### Fixed

- Loading a world chart with no recorded structures no longer copies into a null pointer, which is undefined behaviour even for zero bytes.

## [0.6.3] - 2026-11-03

### Fixed

- Ground warnings under burrowers, hazards and other telegraphed attacks spin again; the ring's tick marks had stopped turning when the ring was redrawn as geometry.
- The landmark pointer no longer reads an unset site when every landmark nearby has been visited.
- Foes slain during a tick are no longer freed while the spatial grid still points at them, so area effects and searches run between ticks can no longer touch a dead foe's memory.

### Changed

- The Windows build is warning-free at `/W4` and treats warnings as errors. Third-party headers are compiled quietly, and shadowed names, narrowing conversions, an uninitialised test value and a hidden virtual overload (`ExpansionEnemy::mend`, now `tend`) are cleaned up.
- Content tables are checked against each other: every hero weapon, skill and favour, evolution pair, upgrade and recipe owner, area-weapon entry and ally recipe must name something that exists, and names must be unique. `--test` runs the check, and Debug builds refuse to start on a broken link. The skeleton and decoy allies dropped stale recipe labels that matched nothing.
- The campaign save format moved to `CampaignSave.cpp` and the campaign session lifecycle to `GameCampaign.cpp`, out of the gameplay and main-loop files.
- AddressSanitizer builds bypass the entity pool so every entity is tracked, and five tests that read entities after they were freed now check by id.
- New CMake switches: `MELLANGAN_WERROR` (on by default with MSVC), `MELLANGAN_ANALYZE` for the MSVC static analyzer, and `MELLANGAN_ASAN` for AddressSanitizer, plus UBSan with GCC and Clang.

## [0.6.2] - 2026-11-03

### Added

- Dirt patches and lakeshores have natural edges: the border wobbles in and out along its length, inside corners curve instead of meeting at right angles, and the fringe no longer repeats every tile.
- A more natural countryside: grass drifts between lush green and sun-dried yellow in broad patches, flowers grow in drifts on the brighter ground instead of an even sprinkle, and grass tiles are mirrored at random so their flowers no longer line up in rows. Every wild tree, bush, flower and rock is mirrored at random and its shade and warmth vary a little, so forests no longer look cloned.

## [0.6.1] - 2026-11-03

### Added

- Fortified towns: every campaign town now has a single wooden bridge through its fence, on a random side, and its other three gates are sealed shut. Foes can no longer cross the walls anywhere else, not even flying ones; they find their way around the fence to the bridge. Shades still drift over walls to reach the lights.
- Town knights: four knights guard each town's bridge from the very first night. They cannot be hurt, and every swing cleaves through all foes in reach and knocks them back. Their blows match the strength of the foes around them, so the garrison can hold a whole night wave alone.
- Dirt patches and lakeshores have natural edges: the border wobbles in and out along its length, inside corners curve instead of meeting at right angles, and the fringe no longer repeats every tile.
- A more natural countryside: grass drifts between lush green and sun-dried yellow in broad patches, flowers grow in drifts on the brighter ground instead of an even sprinkle, and grass tiles are mirrored at random so their flowers no longer line up in rows. Every wild tree, bush, flower and rock is mirrored at random and its shade and warmth vary a little, so forests no longer look cloned.
- Cleaner textures across the generated art: stone, bone, wood and sand show small chipped marks instead of salt-and-pepper speckle, and soft shading turns at an even checker instead of random pixels. The great mushroom's stem and the ancient gate's pillars are rounded, the gate shows brick courses, the shipwreck has a proper sail, and the frozen giant's ice has a sheen.
- Talent badges are bevelled medallions with a lit rim and a dark face, so their symbols stand out.
- The moth wisp has clean wings with eyespots, and the shade is easier to see at night.
- Sharper foe sprites: every bestiary foe is shaded through hue-shifted colour ramps that match the Kenney art, with a rim light and a base shadow. Nine foes are repainted: the cutthroat shows a face and a red mask, the snapjaw has jaws, teeth and back scutes, the boar a bristled back and a separate head, the sand wyrm a toothed maw, and the spore puffer, magma slug, rock crawler, kobold miner and nest have clearer shapes.
- The bastion's honour: foes slain by the knights drop no gems or loot and don't count toward your kills. At night a counter above the town tallies the fallen at the bridge, and at dawn the town pays you for it: "THE BASTION HELD  N FELL AT THE BRIDGE", 1 gold and a little experience per foe.
- The campaign's towns now read as bastions their people refuse to let fall. The townsfolk speak defiantly, and nightfall and dawn are announced as "HOLD THE BRIDGE" and "THE BASTION STILL STANDS".


## [0.6.0] - 2026-11-02


- The mire leech is a segmented, arching leech with a toothed red mouth instead of a small lozenge.
- Foes take steps: every walking foe from the later acts alternates its idle pose with a step frame (a lifted foot, or a squash for the legless) in time with its sway.
- Every generated sprite (trees, plants, rocks, frontier props and creatures, landmarks, the expansion heroes and foes, weapon and relic icons) gets the same Kenney-style finish: colours hue-shifted toward violet in the shadows and warm in the lights, and a lit top edge.
- The townsfolk you find and bring home are now called sanctuary seekers.
- A hound of your own: after you rescue three sanctuary seekers, a grateful hunter gives you his hound. It follows you through the campaign, bites harder and has more health as you level, and bounds back to your side when left behind. If it falls, it limps home and is back at dawn.
- Watchtowers: press B while talking to an act's elder to raise a stone watchtower in that town for 300 gold, 12 stone and 8 wood. It shoots arrows at any foe that comes near, growing stronger with your level, and stands for good.
- Lost sanctuary seekers: by day in the campaign, someone cries for help out in the wilds, and an arrow points the way. Walk up and they follow you; leave them too far behind and they stop and wait. Foes can hurt them on the way. Bring them inside any town and they join its sanctuary seekers, thank you with three materials and make the merchant 5% cheaper, up to a quarter off. Hearthwick grows with each rescue.
- Every campaign town shelters three sanctuary seekers. By day they wander inside the fence; at night they huddle by the fire and shiver. Walk up to one and they speak a line about their act's troubles in a speech bubble.
- The title screen shows a summary of the saved campaign under "K  CONTINUE THE CAMPAIGN": class, level, act and day, plus Hardcore and Nightmare when they apply.
- In the campaign, the world map (Tab) marks your towns by name, with a dot on the current act's town. It also shows the current quest site or the road to the next town as a pulsing ring, your campfires, and the mines you have cleared. A Campaign legend lists the current quest.
- Campaign quests now reward an item as well as gold. Ordinary quests give a magic item, act bosses a rare and the Cinder King a unique. Each act's elder favours a kind of gear: weapons in Hearthwick, off-hands in Saltmoor, armour in Frosthold, charms in Dunewatch and weapons again in Last Ember. The quest dialog names the reward before you accept.
- Clearing an act now ends in a burst of gold rings and sparks.
- Five new kinds of campaign quest:
  - Fetch: in Act II's "The Sunken Reliquary", the reliquary lies sealed in the ruins while the Drowned Abbot keeps it. Slay him, take the reliquary and carry it home to the Abbess. Fall on the way and it slips back to the ruins.
  - Gather: in Act III's "Fuel the Beacons", bring Warden Kell twelve wood.
  - Escort: in Act V's "The Burning Road", Sister Imra carries the last lantern to a shrine. She walks only while you stay close, foes that reach her wear her down, and if she falls the quest must be taken again.
  - Purge: in Act II's "Cleanse the Nests", four spore nests take root in the ruins and keep breeding spore puffers until you destroy them.
  - Defend: in Act IV's "Guard the Caravan", hold the caravan camp for 45 seconds while raiders and hyenas come in waves. The clock only runs while you stay at the camp.
- Forget your path: press R while talking to an act's elder to unlearn your whole skill tree for 25 gold per hero level and get every spent point back. Relics stay.
- Each campaign town wears its act. Saltmoor is hedged with reeds and driftwood, Frosthold with ice blocks around a glowing crystal spire, Dunewatch with cacti, and Last Ember with smouldering obsidian. A wooden plate below each town's south gate carries its name.
- Falling in the campaign wakes you at your nearest campfire when one is closer than your town's waypoint.
- Hold E to keep striking a tree, rock or crystal.
- Shades: on campaign nights, when torches or campfires burn near you, shades drift in. They ignore you and hunt the lights, snuffing torches and smothering fires; a smothered campfire no longer keeps foes from spawning. Press E beside a dead light to relight it: 1 wood for a torch, 2 for a fire. Hearthwick's own lights return each time you come back.
- Torches and campfires now turn most foe spawns away from their surroundings in the campaign.
- Delves: in the campaign, press E at a mine's mouth to delve into it. The mine caves in behind you, crushing whatever stood too close and sealing you inside a ring of rubble. The world fades to near black, so only your light, the mine's lanterns and its fire show the way, while dust sifts down from above. Each new floor shakes the ground harder, and three floors of cave bats, kobolds, rock crawlers and ore golems follow. The deepest floor holds the act's guardian: the Cyclops, the Spore Mother, the Frost Colossus or the Glass Tyrant. A cleared mine stays cleared and leaves a treasure chest, a boss's loot, ore and rune stones. Falling in a delve lifts the cave-in.
- Hardcore campaigns: press R on the campaign's class screen. You get one life, and death deletes the hero.
- Nightmare: after the Cinder King falls, the last elder offers Nightmare. The acts begin again from Hearthwick with every foe much stronger, and your hero, gear and towns are kept.
- The campaign runs through all five acts. Clearing an act's boss opens a road to the next act's town, a new bastion its people refuse to give up, with its own elder, merchant, stash and waypoint:
  - Act II, Saltmoor (the marshes): drain the bog lanterns, recover the reliquary from the Drowned Abbot, cleanse the spore nests, and slay the Bog Witch.
  - Act III, Frosthold (the snow): relight the beacons, slay Old Rimefang, break the lich's servants, and slay the Lich.
  - Act IV, Dunewatch (the dunes): guard the caravans, shatter the Red Mesa, bring down Skyreaver, and slay the Storm Herald.
  - Act V, Last Ember (the ashlands): walk the burning road, open the Hollow Gate, slay Ashmother, and end the Cinder King.
  - An arrow points along the road to each new town. Press I at any waypoint stone to travel between the towns you have reached.
- A first taste of the campaign: press K on the title screen (or start with `--campaign`) to found Hearthwick and keep one hero in one world across sessions. A new campaign asks for your class first; an existing one continues where you left off.
  - Hearthwick is a bastion its people refuse to let fall: a closed fence with its gates barricaded by an overturned cart, crates, barrels, sacks and log piles. Thorn hedges guard the gates, bones and a warning sign lie outside, and smoke and embers rise from a burnt tree and the campfire within. Inside are torches, a waypoint stone, a stash chest, and the merchant with his workbench, who stay in town and stock wares for your level. Foes never spawn inside it.
  - Days are quieter but not empty: a thin trickle of foes roams by day, and the hordes come at night.
  - Days and nights repeat. The clock shows the day and counts down to dusk, then to dawn.
  - Foes grow stronger the further you roam from town.
  - Stepping onto the waypoint saves and heals you. The game also saves every minute, when you close the stash and when you quit.
  - Falling wakes you by the waypoint for a tenth of your gold.
  - The stash holds 32 items. Press I beside the chest to move items between it and your bag.
  - Your class, level, gold, materials, gear, bag, stash and learned upgrades carry over, and so does every change to the world.
  - In the campaign, levelling up gives skill points instead of cards. Press K to open your class's skill tree.
    - The tree has three tabs: Arms (your class weapons first, then the ones your class favours), Arts (skills) and Instincts (talents and passives).
    - Each tab has up to four tiers, opening at hero level 1, 4, 9 and 15, and each tier needs a point in the one above.
    - Skills rank up to five, and every rank after the first shortens the cooldown by a tenth. Switching between learned skills is free.
    - A weapon that is ready to evolve evolves for a point.
    - Treasure chests still offer their evolution and relic cards.
  - Gather and build in the campaign:
    - Press E beside a tree, rock, crystal, flower, fungus, rune stone or bone pile to strike it. After a few strikes it falls and drops wood, stone, shards, herbs, rune stones or bones, and it grows back a few days later.
    - Press B to build a campfire, torch, fence, stone wall or workbench from your materials. A ghost shows where it will stand: green when it fits, red with the reason when it doesn't. Enter builds and Esc cancels.
    - A campfire keeps foes from spawning around it. A placed workbench crafts like the merchant's.
    - Press E on something you built to take it down for half its materials back. Hearthwick's own fence, torches and fire stay put.
    - Wood and stone are two new materials; foes never drop them.
  - Act I has a story. Elder Maren in Hearthwick (a "!" floats over her when she has news) gives four quests in turn; press I beside her to take one or claim its reward:
    - Rekindle the Greenwood: light three shrines.
    - The Barrow Keeper: the miller is held in the nearest graveyard; reach it and slay the Pale Warden.
    - Hunt the Pack: slay twenty wolves.
    - The Pack Lord: find his den at the nearest camp and slay him to clear Act I.
  - A quest line at the bottom left tracks your progress, and an arrow points to a quest site once it is off screen.
- Allies: wolves, hawks, ghouls, skeletons, clockwork drones, powder crews and decoys can fight beside you. They chase foes near you, apply their weapon's effects, take hits, and fade when their time runs out. Foes turn on allies that come close, and decoys draw them away from you. Your attacks never hurt allies. Wolf Totem, Hawk Companion, Ghoul Pack, Clockwork Drone and Powder Crew summon them; `--allies N` brings a mixed pack in dev runs.
- Twelve new weapons any class can draft, each with its own icon and evolution:
  - Bloodletter Fan, Hex Whip, Bone Spear and Thunder Drum.
  - Frost Lantern, Rot Bellows, Grave Bell and Tidal Ring.
  - Ember Wall, Sky Lance, Wolf Totem and Mirror Shards.
- A third signature weapon for every class, each with its own icon and evolution:
  - Mage: Runic Wall. Ranger: Hawk Companion. Knight: Shield Wall. Alchemist: Philosopher's Spiral.
  - Stormcaller: Storm Whip. Reaper: Ghoul Pack. Priestess: Sun Spiral. Duelist: Riposte Whip.
  - Warden: Bramble Wall. Tinkerer: Clockwork Drone. Oracle: Fate Spiral. Corsair: Powder Crew.
- Twenty new skills, each with its own icon.
  - **For every class:** Smoke Bomb, Blood Rite, Gravity Well, Mirror Image, Rallying Horn, Shatter Wave, Second Sight and Sprint.
  - **One more per class:**
    - Mage: Prismatic Prison. Ranger: Call the Hawk. Knight: Last Stand. Alchemist: Transmutation Circle.
    - Stormcaller: Eye of the Storm. Reaper: Raise Dead. Priestess: Halo of Mercy. Duelist: Perfect Parry.
    - Warden: Earthen Wall. Tinkerer: Overclock. Oracle: Foresight. Corsair: Cannonade.
  - Crowned elites can learn seven of them.
- Twenty-four new talents, each with its own badge.
  - **For every class:** Bloodthirst, Glass Cannon, Momentum, Cold Blood, Pyromaniac, Scavenger, Thick Skin, Vengeance, Night Owl, Hunter's Instinct, Overflow and Afterimage.
  - **One more per class:**
    - Mage: Spellweave. Ranger: Volley Master. Knight: Juggernaut. Alchemist: Catalyst.
    - Stormcaller: Static Build. Reaper: Soul Hoard. Priestess: Sanctified. Duelist: Flow.
    - Warden: Rooted. Tinkerer: Spare Parts. Oracle: Prophecy. Corsair: Plunderer.
- Eight new passives: Spellweave Thread, Wide Grip, Swift Quiver, Keen Edge, Fleet Heart, Ember Heart, Collector and Summoner's Seal.
- Sixteen relics that react to the world:
  - Shrinekeeper's Lamp, Cartographer's Quill, Moon Dial and Storm Glass.
  - Snow Globe, Sun Shard, Bog Heart and Hollow Crown.
  - Grave Dirt, Pilgrim's Staff, Tidecaller and Lantern of Ages.
  - Wanderer's Boots, Star Chart, Thunder Idol and Dawnstone.
- Seven new foes of the Marches, each with its own sprite:
  - Bog toads leap onto you and pull you in with their tongues.
  - The drowned rise from water and chill what they touch.
  - Snapjaws wait beneath the surface and lunge at the bank.
  - Strangler vines root and reel in anyone near.
  - Amber beetles shrug off blows from the front.
  - Spore puffers burst into poison beside you.
  - Sunken priests raise the drowned.
- Seven new foes of the Frost, each with its own sprite:
  - Yetis throw staggering snowballs from range, then close in (Snowfields).
  - Ice wraiths drift through anything and chill you when close (Snowfields, Barrows).
  - Barrow wights drain your life with a beam until you step out of reach (Barrows).
  - Crystal golems throw blows from the front back at you as shards and break into three crystal shards when they fall (Crystal Fields).
  - Moon hounds hunt in packs of four by night, blink to your flanks and fade at dawn (Moonlit Grove).
  - Lynxes wait hidden among the trees, their eyes glinting, and pounce when you pass (Taiga).
  - Barrow archers loose chilling arrows (Barrows).
- Seven new foes of the Scorched lands, each with its own sprite:
  - The sand wyrm, a desert mini-boss, erupts three times under where you stood, then surfaces and takes extra damage before it dives again. Only one hunts at a time.
  - Hyenas run in packs of six and bolt when badly hurt, then come back (Savanna).
  - Salt golems leave a crust behind them that slows you (Salt Flats).
  - Glass spitters spit three shards that ricochet once off walls (Glass Desert).
  - Storm kites circle overhead and call lightning down where you stand (Storm Steppe).
  - Dune raiders throw one spear, then close in with a blade (Desert, Badlands).
  - Totem shamans plant up to two war totems that hasten and ward the foes around them; break the totems first (Savanna).
- Seven new foes of the Burning lands, each with its own sprite:
  - Magma slugs leave a burning trail that lasts four seconds (Ashlands).
  - Ash wraiths stay veiled and hard to hurt until you are close, and their touch burns (Ember Fen).
  - Cave horrors sleep, blind, until fighting breaks out nearby, then hunt the noise (Hollow Caverns).
  - Star spawn split into three starlets when they fall (Starfall Basin).
  - Flame cultists call down a meteor on where you stand two seconds later (Ashlands).
  - Hollow sentinels raise a shield that turns slowly, so circle them to strike the back (Hollow Caverns).
  - The ember drake, a mini-boss, circles overhead breathing cones of fire, then lands to rest every ten seconds and takes extra damage while grounded (Ember Fen).
- Six wandering and underground foes, each with its own sprite:
  - Night stalkers can turn up anywhere at night. Only their eyes show until you hit them, and they are gone at dawn.
  - Grave robbers lurk around graveyards, dig skeletons out of the ground and drop extra gold.
  - Cave bats swarm in twelves (Hollow Caverns).
  - Rock crawlers lie still as stones and drop onto you when you pass (Hollow Caverns).
  - Ore golems are slow and always drop ore (Hollow Caverns, Badlands).
  - Kobold miners throw pickaxes, steal ore from you and run off; slay one to get the ore back (Hollow Caverns, Badlands).
- Ten named packs, one per region, rise the first time you enter their home. Each has a fixed name, two fixed traits and its own followers, and always drops a rare item:
  - Bramblemaw (Meadows) and Grizzlegut the Goblin King (Heath).
  - Mother Mire (Mire) and the Drowned Abbot (Sunken Ruins).
  - Old Rimefang (Snowfields) and the Pale Warden (Barrows).
  - The Red Mesa (Salt Flats) and Skyreaver (Storm Steppe).
  - Ashmother (Ember Fen) and the Hollow King (Hollow Caverns).
- A wandering merchant (in runs with loot): once you light a shrine, he camps by its fire with a lantern and a barrel. Walk up and press I to trade.
  - He sells healing draughts, plain bases and two or three magic items. His wares are fresh at every newly lit shrine and again at dawn.
  - He buys anything from your bag for a quarter of its worth.
  - Gamble: pay six times a base's worth for a mystery item of that base. It always rolls magic or better, with rares twice and uniques three times as likely.
  - His prices climb steeply as the night deepens, so early buys are cheap and gambling becomes the better deal later on.
- A workbench at the merchant's camp. Walk up to it and press I to craft:
  - Foes now drop six crafting materials: crystal shards, ore, rune stones, herbs, hides and bones. Small foes leave shards and herbs, brutes leave ore, and elites and bosses leave rune stones.
  - Reroll the weakest affix of an item, reforge every affix of a rare while it keeps its name, or raise a Worn or Fine item one tier.
  - Combine three magic items of one slot into a rare, or cleanse a night-touched item of its curse; the curse goes, and so does the night's gift.
  - Eight crafted recipes turn a plain item into an orange crafted one with two fixed affixes and up to three random ones: Bloodfang, Stormbrand, Ember Lamp, Sage's Ward, Frostmantle, Thornhide, Blood Charm and Fortune Charm.
- Twelve named unique items with fixed powers, such as Ember Locket, Pack Banner, Last Light and Bone Dice. Unique drops now roll as real uniques instead of rares.

### Changed

- Off-screen pointer labels at the same screen edge now stack instead of drawing over each other.
- Evolved weapons now survive saving and reloading a campaign; before, they were lost on every reload.
- Wall weapons (Ember Wall, Runic Wall, Bramble Wall and the rest) now raise a thick, glowing band with two rows of flickering tongues instead of a thin dotted line.
- The Fleet Heart, Ember Heart and Bog Heart icons are drawn as clear hearts, with speed lines, a flame and a drip.
- The README lists the campaign's keys and explains how the campaign works.
- Stagger no longer knocks foes away from walls, so wall weapons keep holding and hurting them.
- The eight expansion weapons now evolve:
  - Meteor Sling, Leviathan Harpoon, Astral Sigil and Twin Crescent.
  - Siege Battery, Worldvine Lash, Spectrum Lance and Ancestor Host.

  Each evolved form has its own twist: bursting bounces, a rune that rides with you, a lash that cracks both ways, a fan of three beams and more.
- Boomerang marks what it hits and bursts where it turns, hitting marked foes harder.
- Spirit Swarm spirits drag the foes they marked together when they fade.
- Mortar shells stagger instead of burning and leave a crater that slows foes.
- Conduit now gives +15% chain and arc damage instead of a second copy of Keen Eye.
- Foe families now lean their loot: goblins drop more gold; golems, kobolds and sentinels drop ore and armour; cultists, priests and shamans drop off-hands; wraiths and wights drop charms; boars, wolves, hyenas, lynxes, hounds and yetis drop hides and bones; mimics, grave robbers and treasure thieves drop more gold and only magic or better items.
- Bloodfang, Frostmantle, Thornhide and Blood Charm now call for hides and bones.
- The boss bar now also appears for leaders that join a run at its very start.
- The inventory shows how many of each crafting material you carry.
- The Codex bestiary now pages through every foe; foes past the 26th never showed before.
- Foes drop potions about half as often, and gold drops are half as large. Item tooltips now show the merchant's real buying price.

## [0.5.15] - 2026-11-01

### Added

- `BiomeViewer`, a separate tool that shows the whole generated world for any seed, with refresh, zoom, structure sites, copy to clipboard and PNG export.
- Three new statuses, Bleed (stacks, hurts more while moving), Weaken (less damage dealt) and Fear (foes flee), with HUD and body tints, and five new weapon payloads: bleed, weaken, fear, shatter (bonus against chilled foes) and execute (finishes weak non-boss foes).
- Seven new Greenwood foes with their own sprites: thornlings that leave slowing brambles, charging wild boars, goblin archers that loose volleys, goblin shamans that heal goblins and weaken you, saplings that grow into treants, moth wisps that circle, dive and dim your light, and cutthroats that strike harder from behind. They roam the Meadows, Greenwood, Heath, Autumn Woods and Blossom Grove.
- Eight new champion traits: cursed, silencing, fire, cold and storm enchanted (with death novas), stone skin, multishot and aura bearer.
- Three new weapon forms for upcoming weapons: whips that crack harder at the tip, walls that hold foes back, and spirals.
- Run rules for future modes: day and night cycles, spawn and horde scaling, safe zones, fixed omens and threat by place. The dawn run is unchanged.
- Diablo-style loot on the ground behind the `--loot` dev flag: items pop out of fallen foes, rares and better cast a light beam, names show in their rarity colour and stack without overlapping, Alt shows every label, and walking over an item bags it.
- An inventory screen on `I`: a paper doll with weapon, off-hand, armour and two charm slots, a 32-slot bag, Diablo-style tooltips with affixes, curses, class requirements and a green and red comparison against what you wear, and dropping items back on the ground.
- Equipped items now count: affixes raise damage, life, speed, armour, resistances, block, thorns, life leech, magic and gold find, experience, light, projectiles, area and projectile speed, add on-hit burn, chill, poison, bleed and shock, and boost damage against bosses, champions or at night. A weapon item grants its ability at tier level, and taking gear off restores everything exactly.
- The groundwork of a Diablo-style item system: item bases from every weapon plus off-hands, armour and charms; 30 affix families in five tiers; seeded magic, rare and night-touched rolls with magic find; treasure classes per source; item save lines; and `--loot-sim` to check drop rates. Not yet visible in play.

### Changed

- Champion edge pointers on the HUD now work everywhere; they compared world positions against the origin and only showed near the start.
- Reorganised the source for readability: tests and QA views moved to `src/test`, the HUD and save files left `Game`, weapon forms became one class each, and the largest enemy, level and ability files were split by responsibility. Gameplay is unchanged.
- World generation moved into `src/level/world`, one file per concern, with a single tile-memo helper; feature props are one-line table entries and the dev start flags live in `DevStart`. Generated worlds are bit-identical.
- Split the weather into fronts, footprints, thunder and rendering, and structure layouts into settlements, wilds and landmarks; props gained one-call sprite, collider and light helpers. The class and level-up menus now share one glyph renderer and word wrap, with the hero card and upgrade card as their own classes, and `Mob::upgrade` replaces six hand-written name lookups. Generated worlds remain bit-identical.
- Expansion weapons are one table row per kind with named kinds instead of numbered branches; floor painting moved into `FloorPainter`, prop light into `Prop::shine`, and encounters split into sites and landmarks with a shared radius-damage helper.
- The HUD draws one panel per method with shared edge-pointer helpers, the spawner runs one method per schedule, `Level::quake` replaces four hand-written radius damage loops, `world::biomeUnder` replaces eleven pixel-to-biome conversions, and the four enemy worker passes share one template.
- Expansion foes use named variants and one stats row each instead of numbered branches; the camera is its own `Camera`; `RecipeShot`, the Mage skills and the skill arts have their own files with a shared `castRecipe`; biome events split scheduling, forces and pulses.
- The Warden, Tinkerer, Oracle and Corsair are their own hero classes like the other eight; passives and class passives are data tables; `main` hands QA views to one launcher and the dev flag help lives in `DevStart`.
- The title menu draws one section per method with a shared heading helper, the run recap computes its rank and formats its figures in small helpers, and the F8 world debug overlay is its own `WorldDebug` class instead of a 160-line `Level` method.

## [0.5.14] - 2026-11-01

### Changed

- Hills are shaped tile by tile instead of in 3 × 3 blocks, so their cliffs wind and bend like natural contours rather than running dead straight along a grid. Lakes that meet a hill follow its curve too.
- Big hills now rise to a third terrace, a rocky hilltop above the two lower ones, and each terrace stays at least a tile wide.
- Cliffs cast a soft shadow onto the ground below and to their east, so the high ground reads more clearly.
- Stairs only appear on a straight three-tile stretch of a cliff, so they always line up.
- Buildings check that their whole footprint is level, so a cliff never cuts through a cottage or a shrine.
- Climbing changes the land: forest thins on the upper terraces and hilltops open into rocky crowns of boulders, pebbles and heather. The rocks match the region: sandstone in the deserts, obsidian in the ashlands, snow-capped in the north.
- Scree and fallen boulders gather at the foot of cliffs.
- Small mountain tarns sit in the hollows of wide plateaus, frozen over in winter lands.

### Added

- Every class now has a fuller kit, so the Mage is no longer the only one with real choices:
  - **22 new class weapons**, two per class, offered only to that class:
    - Knight: Lionheart Cleave and Bastion Shields.
    - Ranger: Hawk Talon and Bear Trap.
    - Alchemist: Caustic Flask and Witchfire Phial.
    - Stormcaller: Ball Lightning and Arc Lance.
    - Reaper: Soul Harvest and Bone Wheel.
    - Priestess: Lance of Dawn and Seraph Wings.
    - Duelist: Crescent Cut and Flicker Darts.
    - Warden: Thornwhip and Root Snare.
    - Tinkerer: Clockwork Mine and Flash Grenade.
    - Oracle: Prophecy Ray and Fate Wheel.
    - Corsair: Chain Shot and Riptide.
  - **New ways to attack:**
    - melee cleaves that sweep the space in front of you;
    - boomerangs that fly out and come back;
    - lobbed flasks and grenades;
    - traps that spring when a foe steps on them;
    - beams that strike a whole line of foes at once.
  - **New weapon effects:**
    - lifesteal heals you a little with every hit;
    - chain hits arc on to a nearby foe;
    - stagger hits knock foes back hard and pin them for a moment.
  - **11 new class skills**, one per class:
    - War Banner (Knight).
    - Hunter's Mark (Ranger).
    - Flask Barrage (Alchemist).
    - Tempest (Stormcaller).
    - Blood Pact (Reaper).
    - Hallowed Ground (Priestess).
    - Flash Step (Duelist).
    - Overgrowth (Warden).
    - Scrap Barrage (Tinkerer).
    - Eclipse (Oracle).
    - Whirlpool (Corsair).
  - **A class passive** for each of the Warden, Tinkerer, Oracle and Corsair: Deep Roots, Fine Gearing, Farsight and Sea Legs.
- **Class talents** for the Warden, Tinkerer, Oracle and Corsair:
  - **Heartwood:** skills heal you and root nearby foes.
  - **Shrapnel:** slain foes may burst into shrapnel.
  - **Starsight:** every few seconds, foresees and marks the nearest foes.
  - **Plunder:** kills may spill an extra gem.
- **22 new evolutions**, one for every new class weapon. Max out the weapon while holding its partner passive, and it evolves with double damage, faster attacks and an extra strike (or a wider area):
  - Lionheart Cleave becomes Kingsbane Cleave with Oathbound, and Hawk Talon becomes Falcon's Return with Fleet Foot.
  - Each class's first weapon pairs with that class's own passive, and its second with a shared passive.
- The Mage's seven spell weapons can now evolve too, for example Ember Lance with Arcane Mind becomes Sunspear and Flame Ring with Troll Blood becomes Phoenix Ring.
- `--test-arsenal-view CLASS` plays as that class with its whole kit maxed and shows live damage per second for each weapon, for balance checks.
- Skill-specific talents (Beacon, Echoing Blink, Overcharge) now trigger only when you cast your class's own skill, not any drafted skill.
- Skilled elite foes can now also plant banners, loose marking arrows, drain life, grow thorny ground and cast a following eclipse.

### Fixed

- Lakes beside cottages, camps and other buildings no longer stop along a ruler-straight line two tiles out. They now curve around buildings like any other shore, so a narrow strait between two buildings is no longer a perfect rectangle of water.
- The clearing around the starting point is no longer a square hole in the woods. Trees and bushes thin out gradually toward an irregular open circle.
- Ruins no longer sit on a solid rectangle of dirt; each one has an irregular patch of trodden earth.
- The starting area could sit on an almost perfectly square island, because lakes were simply cut away from a fixed square around the spawn. The spawn now has a soft, irregular dry zone instead, so nearby lakes curve naturally around it and the start usually joins the mainland.

## [0.5.13] - 2026-10-31

### Added

- Five new foes:
  - **Mimic.** From about a minute in, a chest may wait in the grass ahead of you. Weapons ignore it, but it rattles as you draw close and springs at you if you step within reach. It bites hard, dies in a shower of coins and often leaves a potion.
  - **Renegade adventurers.** From about three minutes in, small bands of rogue adventurers hunt you, announced by "RENEGADES ON YOUR TRAIL". They carry the same weapons you do, and before each special move they glow and flash rings for a moment, so you can step aside:
    - the **Outlaw** keeps its distance with a Hunter's Bow and lobs powder kegs at your feet;
    - the **Berserker** hurls axes and makes shield charges;
    - the **Pit Fighter** circles with orbiting knives and lunges;
    - the **Deserter** wears a warding halo and rallies with a sweeping cut behind a ward.

    Renegades often drop a potion.
- Eleven new class skills, one more for every class that had only one. They appear on level-up from level 4, like the Mage's extra skills:
  - Ranger, **Arrow Storm**: sixteen arrows in every direction;
  - Knight, **Shield Charge**: charge ahead, battering every foe in your path;
  - Alchemist, **Miasma**: poison every foe within reach for five seconds;
  - Stormcaller, **Chain Storm**: lightning leaps between eight nearby foes;
  - Reaper, **Death Grip**: drag nearby foes to you and chill them;
  - Priestess, **Sacred Light**: pillars of light fall on six foes;
  - Duelist, **Lunge**: a long, piercing thrust;
  - Warden, **Brambles**: root nearby foes in thorns;
  - Tinkerer, **Minefield**: a ring of mines around you;
  - Oracle, **Comet**: three comets on the nearest foes;
  - Corsair, **Powder Keg**: a keg that bursts into a wide fire.
- Eight general skills that every class can learn, so a skill offer is no longer mostly the Mage's:
  - **War Cry**: hurl back nearby foes; they take 30% more damage for five seconds;
  - **Bandage**: heal 30 and shake off burns, poison and chill;
  - **Tumble**: roll ahead, untouchable for a moment;
  - **Whirlwind**: spin, cutting every foe around you;
  - **Ground Slam**: shake the earth, damaging and stunning nearby foes;
  - **Fire Ring**: ring yourself in flame for four seconds;
  - **Lodestar**: draw in every gem nearby;
  - **Snare Net**: bind the foes ahead of you for three seconds.

- Seven new omens, and most runs now begin under one (about 85%, up from 20%). Each run reads differently:
  - **Outlaw Season**: renegades ride early and often;
  - **Mimic Moon**: more mimics, and gems run richer;
  - **The Swarm**: twice the horde, half as hardy;
  - **Iron Night**: fewer foes, far tougher, more elites;
  - **War Drums**: elites are common, and every one knows a skill;
  - **Long Night**: waves come thick and fast;
  - **Quiet Hours**: gentle waves and fewer elites, but leaner experience.
- The minute wave is no longer always the same ring of the strongest foe. It now rotates through six kinds, never the same twice in a row:
  - **The Horde Closes In**: the classic ring;
  - **A Swarm Descends**: a large ring of weak foes guarding two elites;
  - **The Pack Hunts**: alpha wolves and their packs from one side;
  - **Ambush!**: a wall of mixed foes across your path;
  - **The Dead Rise**: skeletons climb out close around you, with a necromancer behind them;
  - **The Land Stirs**: a ring of the foes native to the land you stand in.
- Elites can know a skill: War Cry, Ground Slam, Snare Net, Tumble, Fire Ring, Whirlwind, Bandage or Miasma. About a third of elites have one, and all of them do under War Drums. They glow and flash rings before casting, like renegades.
- Bosses have wider rotations, and every boss cast now shows the same warning:
  - the Lich adds Comet;
  - the Pack Lord adds War Cry;
  - the Cyclops casts Volley and Ground Slam.

- Champions: named foes that walk the land. From about two minutes in, a champion may appear, announced as "A CHAMPION STALKS: MORGRA THE HOLLOW". Each one is built at random, so no two are alike:
  - a foe native to the land, with a crown ring and a golden glow;
  - two traits, one general skill and a real weapon from your arsenal;
  - its name on the boss bar, and a gold arrow with its name and distance when it is off screen.

  Only one walks at a time. Each always drops a chest, and your death screen names it if it fells you.
- A new omen, **Champion's Hour**: champions come far more often.
- Six new bosses, one for each family of lands. A boss now usually matches the land you stand in, and the classic three still turn up anywhere:
  - **The Frost Colossus** (snow, taiga): slams, nets and icy comets, with frost wisps;
  - **The Glass Tyrant** (deserts, badlands, salt flats, glass desert): lunges, charges and fans of glass shards, with dust vultures;
  - **The Spore Mother** (fungal wood, caverns, jungle): poison clouds, roots and a drag that pulls you in, with sporelings;
  - **The Cinder King** (ashlands, ember fen): powder kegs, rings of fire, falling comets and mines, with ember imps;
  - **The Bog Witch** (mire, floodplain): drags you in through poison and calls pillars of light, with mire leeches;
  - **The Storm Herald** (storm steppe, starfall basin, crystal fields): thunder, chain lightning and comets, with cultists.
- Every boss now fights in three phases, including the Cyclops, the Pack Lord and the Lich. At two thirds and one third health it enrages: it calls minions, raises a brief ward and switches to a harsher set of attacks. Two marks on the boss bar show where each phase begins. Bosses also burst into gems when they fall.

- Six new elite traits, doubling the pool that elites and champions draw from. Each has a mark and a counter:
  - **Thorned** (spiky olive ring): hitting it up close stings back, so fight it from range;
  - **Regenerating** (green): heals once it has been left alone for a moment, so keep the pressure on;
  - **Blinking** (violet): jumps away when struck, but blasts and auras still reach it;
  - **Phasing** (pale): turns translucent and nearly untouchable for one second in every four, so time your strikes;
  - **Brooding** (amber): sheds weak copies of itself every eight seconds, so kill it fast;
  - **Gravitic** (indigo, pulsing ring): slowly drags you toward it, so keep moving.

### Fixed

- The boss bar always shows the real boss, even when a champion is also on the field.
- Off-screen markers (shrine, landmark, champion) no longer overlap the boss bar.
- A failed test now prints its error and exits instead of opening a dialog that blocks scripted runs.
- In Release builds, tests now actually check their assertions. One test used to spin forever there, which left a black window open and locked the build.

### Changed

- From level 4, a level-up offers a skill a little more often (one time in four, up from one in five). The offer draws from your class's skills and the general ones.
- A foe given a single weapon now carries only that weapon, instead of a full copy of every weapon in the game.

## [0.5.12] - 2026-10-31

### Added

- Landmarks: rare, towering sights that give you a reason to travel. About every second stretch of 80 by 80 tiles holds one, never near the start, always on level, dry ground, with a light you can spot at night and a marker on your map once sighted. Each has a first-visit reward:
  - **The Ancient Tree**, a vast lantern-fruit oak among forests and meadows. Shelter under it to heal fully and gain 15 maximum health. Leaves and fireflies drift from its crown.
  - **A Fallen Star**, half sunk in a scorched crater. Your arrival releases its light: a blast that throws foes back, a shower of gems and a chest.
  - **The Colossus**, the skull and ribs of something enormous. Its dead rise around you, guarding a chest.
  - **The Monolith**, a black obelisk carved with glowing runes on an old plaza. It charts the land around it on your map and tells a fragment of lore.
  - **The Crystal Spire**, a towering cluster of amethyst in crystal lands. It rings with power: your class skill is ready at once, you gain 5% might, and a shard burst throws foes back.
  - **The Great Mushroom**, a red-capped giant of the autumn, mire and fungal woods. Its spores mend you (+0.4 health per second for the run) and poison the foes around you.
  - **A Shipwreck**, a hull stranded in deserts, salt flats and floodplains. Two chests of plunder and a scatter of gems.
  - **The Ancient Gate**, a stone arch around a swirling portal. Step through and it carries you to the nearest shrine you have not yet kindled.
  - **The Ember Forge**, an anvil over glowing coals in the ash and badlands. It tempers one of your weapons by a level.
  - **The Frozen Giant**, a figure sealed in ice in the snowfields and taiga. The cold hardens your skin (+0.5 armour) and freezes nearby foes in place.

  Each land favours its own kind, but any landmark can turn up anywhere.
- Landmarks can be spotted from afar. The nearest landmark you have not yet visited, within about 70 tiles, shows a small marker at the edge of the screen with its name and distance, in its own colour and brighter as you approach. The marker disappears once the landmark is on screen, and markers stay clear of the top display.
- Elite foes now carry a trait instead of a plain gold tint. Each shows as a coloured ring and a matching glow, and each has a counter:
  - **Hasted** (pale yellow): 40% faster. Keep your distance.
  - **Vampiric** (red): heals when it hits you. Kill it quickly.
  - **Warded** (blue, double ring): its ward soaks most damage for five hits, then breaks with a burst. Hit it often.
  - **Splitting** (green): bursts into two smaller copies when it dies. Clear space first.
  - **Volatile** (flickering orange): explodes shortly after death, with a telegraphed circle. Step out.
  - **Frosted** (ice blue): leaves a freezing trail that chills you. Don't follow its path.
- Bosses now use class skills. Every few seconds the Lich blinks toward you in an arcane burst, as the Mage does. The Cyclops looses a fan of seven arrows, like the Ranger's Volley. The Pack Lord either unleashes a nova that throws you back and mends it a little, or dashes at you cutting on both sides like the Duelist. The Lich sometimes calls lightning down around you instead of blinking.
- Traits are not only for foes: the Frozen Giant now also gives you the Frosted trait, so you leave a freezing trail that chills foes that walk your path.
- High ground matters. Your attacks deal 15% more damage to foes standing below you, so a plateau is worth holding (flyers excepted).
- Dev: `--landmark N` starts a `--dev` run beside the nearest landmark of kind N (0 tree, 1 star, 2 colossus, 3 monolith, 4 spire, 5 mushroom, 6 wreck, 7 gate, 8 forge, 9 frozen giant).

### Changed

- Under the hood, every weapon, talent, relic and class skill now works for any creature, not only for you. A foe given a weapon fires it at you and its attacks spare other foes. This lays the ground for armed elites and bosses; for now, normal play is unchanged.

- Announcements are half the size, on a slimmer plate, so they cover less of the fight.

- Cliffs now match their land:
  - ice in the Snowfields, the Taiga and Crystal Fields;
  - sandstone in deserts, badlands and salt flats;
  - basalt in the Ashlands, the Ember Fen and the Hollow Caverns;
  - mossy stone in green woods, mires and jungles;
  - grey stone in barrows, heaths and ruins.

  Meadows keep their earthen walls. Each material has a lit west side, a shaded east side and a striated south face. A stretch of cliff keeps one material even where two lands meet.
- Ramps are now real stairs. Every few blocks along a plateau's south face, a flight of steps three tiles wide climbs to the top, built from the cliff's own stone (or timber in the meadows), with side walls at its ends. Walk up them at full speed, while scrambling up a bare cliff stays slow. Trees and rocks never block a stairway.
- Dev: `--cliff` combined with `--biome N` starts below a cliff inside that biome, and `--ramp` starts at the foot of the nearest stairs.

### Fixed

- Landmarks no longer look fractured. Their upper parts were drawn from the wrong rows of the sprite sheet, mixing pieces of different landmarks. Tall landmarks also no longer pop out of view when their base is just below the screen edge, and the Colossus and Shipwreck no longer sit on scattered dirt squares.

- The class card on the class picker no longer runs off the bottom of the screen. Each class's arsenal is laid out in two compact columns, with talents in the class colour. A class with a very large arsenal shows it as a grid of icons.

- Arsenal icons drew as lake-shore tiles. The arsenal and the new lake and cliff tiles had both claimed the same part of the sprite sheet when they were merged; the arsenal now has its own space.

## [0.5.11] - 2026-10-31

### Added

- The Mage gains seven signature weapons (Ember Lance, Hoarfrost Fan, Arcane Orbit, Starfall, Void Rift, Seeker Runes, Flame Ring) and four new skills (Time Warp, Mana Surge, Arcane Missiles, Singularity), each with its own icon and effects.
- Level-ups can now offer a swap to another of your class's skills; every class's original skill can be drafted back.

## [0.5.10] - 2026-10-31

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

- Lakes. Ponds and lakes dot the world, gathered in hollows rather than scattered as puddles:
  - The Mire, the Floodplain and the Sunken Ruins are rich in water; deserts, badlands and the Ashlands stay dry.
  - Lakes keep clear of the start and of every building site, so shrines are always on dry land.
  - Each land rings its lakes with its own ground (ash, moss, clay and so on).
  - Lakes in the Snowfields freeze into ice.
- Swimming:
  - You can wade into any lake and swim at a little over half speed, with a floaty glide, sunk to the waist with a wake around you.
  - Ice is slippery.
  - Foes swim slowly too, except Mire Leeches, which are faster in water; bats, ghosts and other flyers pass straight over. A lake is a place to break a chase, or a trap if the wrong foes follow.
  - Water glints in your torchlight, rain rings on lakes, and lakes mirror what stands in and beside them.
- Cliffs and plateaus. The land now rises in broad, blocky plateaus, up to two levels high, edged by long, even earthen cliffs with clean corners:
  - A plateau's south face shows as a tall wall casting a short shadow, and its other edges as rocky lips.
  - Higher ground is lit a little brighter, so height reads at a glance.
  - Climbing a cliff is a slow scramble for you and for every walking foe, while ramps (three-tile openings in the south faces) and jumping down cost nothing. High ground buys you a moment.
  - Flyers ignore height.
  - Lakes only form in the lowlands, and building sites only stand on level ground, never across a cliff.
- Dev: `--at X,Y` starts a `--dev` run on a tile, `--lake` at the nearest lakeshore, and `--cliff` just below the nearest cliff.

### Changed

- Region events are defined in one table, which makes each event easy to tune.
- The world is calmer. Biomes now form large, coherent regions about four screens across, with organic borders, instead of a patchwork of thin strips and stray patches. The ten newer biomes form their own wild lands rather than ribbons running through everything else, and the dark lands (Ashlands, Crystal Fields, Fungal Wood and Barrows) no longer have green meadow holes inside them. The ten newer biomes also no longer scatter grass-rimmed dirt patches across their ground. Mines, ruins and camps in lands without grass no longer sit in a green frame: their dirt yards now blend into the local ground (ash, crystal moss, mycelium, red clay, needles, straw and each of the ten newer grounds), and the dirt itself takes on that land's tone. The map has about 92% fewer biome borders, and the start area stays gentle. World layouts differ from earlier versions.

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
