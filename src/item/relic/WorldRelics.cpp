#include "item/relic/WorldRelics.hpp"

#include "audio/Sound.hpp"
#include "entity/Ability.hpp"
#include "entity/Ally.hpp"
#include "entity/Player.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

using S = ItemStats;

struct CharmDef {
  const char *name, *desc;
  float S::*stat;
  float amount;
};

const CharmDef CHARMS[] = {
    {"Shrinekeeper's Lamp", "+20% damage for 20s after kindling a shrine"},
    {"Cartographer's Quill", "Every new region you enter heals 10"},
    {"Moon Dial", "Weapons fire 25% faster in the night's last three minutes"},
    {"Storm Glass", "In rain or storm, your crits call chain lightning"},
    {"Snow Globe", "+2 armour in snowy lands"},
    {"Sun Shard", "In the deserts your hits may burn"},
    {"Bog Heart", "In the mires: immune to poison, regenerate 0.5/s"},
    {"Hollow Crown", "+3% damage for every champion you slay"},
    {"Grave Dirt", "Graveyards raise ghosts to fight beside you"},
    {"Pilgrim's Staff", "+1% speed for every landmark you have visited"},
    {"Tidecaller", "Standing in water heals 3/s"},
    {"Lantern of Ages", "The shrine beacon reaches one region further", &S::lantern, 1},
    {"Wanderer's Boots", "+5% speed for every land you have seen, up to +25%"},
    {"Star Chart", "The minimap shows nearby landmarks", &S::starChart, 1},
    {"Thunder Idol", "Every 60s a thunderclap strikes six foes"},
    {"Dawnstone", "Every night ends 30s sooner"},
    {"Ember Locket", "Rise once from death in embers"},
    {"Hearthstone", "Below 25% health, return to your last lit shrine; 90s cooldown"},
    {"Shrine Key", "The first shrine each night kindles 50% faster"},
    {"Pack Banner", "A spirit wolf fights at your side and returns when it falls"},
    {"Sea Charm", "Standing in water heals 2/s"},
    {"Map of Old Roads", "The minimap shows nearby landmarks", &S::starChart, 1},
    {"Bone Dice", "Champions drop an extra handful of loot", &S::extraLoot, 1},
    {"Last Light", "Once a night, a killing blow leaves you at 1 health, warded for 3s",
     &S::lastLight, 1},
};
static_assert(std::size(CHARMS) == size_t(Charm::Count), "every Charm needs a row");

constexpr Color GOLD{1, 0.85f, 0.45f, 1}, TIDE{0.4f, 0.85f, 0.95f, 1};

bool among(BiomeId b, std::initializer_list<BiomeId> set) {
  for (BiomeId s : set)
    if (s == b)
      return true;
  return false;
}

} // namespace

WorldRelic::WorldRelic(Charm kind)
    : Upgrade(CHARMS[int(kind)].name, atlas::arsenal(ICONS + std::min(int(kind), RELICS - 1)),
              {}, false),
      kind(kind) {
  const char *d = CHARMS[int(kind)].desc;
  desc = {d, d, d, d, d};
  rare = true;
  hidden = int(kind) >= RELICS;
}

void WorldRelic::levelUp(Mob &p) {
  Upgrade::levelUp(p);
  const CharmDef &d = CHARMS[int(kind)];
  if (d.stat && !sealed)
    p.bonus.*d.stat += d.amount, sealed = true;
  Player *pl = p.asPlayer();
  if (kind == Charm::Dawnstone && pl && !given)
    p.level->rules.nightLength -= 30, given = true;
  if (kind == Charm::EmberLocket && pl && !given)
    pl->revives++, given = true;
}

void WorldRelic::forget(Mob &p) {
  const CharmDef &d = CHARMS[int(kind)];
  if (d.stat && sealed)
    p.bonus.*d.stat -= d.amount, sealed = false;
  if (held)
    p.*held -= applied, applied = 0;
  if (lent)
    p.bonus.*lent -= loaned, loaned = 0;
  p.might -= 0.03f * crowns, crowns = 0;
  p.status.immune &= ~(1u << int(StatusKind::Poison));
  if (Mob *wolf = p.level->mob(pet))
    if (auto *a = dynamic_cast<Ally *>(wolf))
      a->retire();
  pet = entt::null;
}

void WorldRelic::hold(Mob &p, float Mob::*stat, float want) {
  held = stat;
  p.*stat += want - applied;
  applied = want;
}

void WorldRelic::lend(Mob &p, float S::*stat, float want) {
  lent = stat;
  p.bonus.*stat += want - loaned;
  loaned = want;
}

void WorldRelic::tick(Mob &p) {
  Player *pl = p.asPlayer();
  if (!pl)
    return;
  Level &level = *p.level;
  BiomeId biome = world::biomeUnder(p.x, p.y);
  cd = std::max(0.0f, cd - DT), timer = std::max(0.0f, timer - DT);
  switch (kind) {
  case Charm::ShrineLamp:
    if (level.night.lit > lit)
      timer = 20, Spark::burst(level, p.x, p.bodyY(), GOLD, 12, 80, true);
    lit = level.night.lit;
    return hold(p, &Mob::might, timer > 0 ? 0.2f : 0);
  case Charm::Quill: {
    int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16));
    int64_t key = int64_t(world::regionOf(tx)) << 32 | uint32_t(world::regionOf(ty));
    if (regions.insert(key).second && regions.size() > 1)
      p.mend(10);
    return;
  }
  case Charm::MoonDial:
    return hold(p, &Mob::haste, !level.night.dawn && level.night.left() < 180 ? -0.25f : 0);
  case Charm::StormGlass:
    if (pl->crits != seen && level.sky.rainfall() > 0.2f && cd <= 0)
      cd = 0.4f, ability::chain(p, 2, 160, 12);
    seen = pl->crits;
    return;
  case Charm::SnowGlobe:
    return hold(p, &Mob::armor, among(biome, {BIOME_SNOW, BIOME_TAIGA}) ? 2 : 0);
  case Charm::SunShard:
    return lend(p, &S::burn,
                among(biome, {BIOME_DESERT, BIOME_SALT, BIOME_BADLANDS}) ? 25 : 0);
  case Charm::BogHeart: {
    bool bog = among(biome, {BIOME_MIRE, BIOME_FLOOD});
    if (bog)
      p.status.immune |= 1u << int(StatusKind::Poison);
    else
      p.status.immune &= ~(1u << int(StatusKind::Poison));
    return hold(p, &Mob::regen, bog ? 0.5f : 0);
  }
  case Charm::GraveDirt: {
    Structure s;
    int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16));
    if (cd > 0 || !world::structureAt(tx, ty, s, 2) || s.kind != StructureKind::Graveyard)
      return;
    cd = 12;
    for (int i = 0; i < 2; i++)
      summon(level, AllyKind::Ghost, p.x + (i ? 20 : -20), p.y + 8, 14 * p.might, 12);
    flourish::sigil(level, p.x, p.bodyY(), 30, Color{0.7f, 0.85f, 1, 1}, 6);
    return Sound::raise.play(0.4f, 0, 1.3f);
  }
  case Charm::PilgrimStaff:
    return hold(p, &Mob::speed, 0.74f * level.landmarks);
  case Charm::Tidecaller:
  case Charm::SeaCharm:
    if (level.swimming(&p)) {
      p.hp = std::min(p.maxHp, p.hp + (kind == Charm::Tidecaller ? 3 : 2) * DT);
      if (rng.chance(0.1f) && Spark::alive < 600)
        level.add<Spark>(p.x, p.bodyY(), 0, -30, TIDE, 0.4f, 0.4f, true);
    }
    return;
  case Charm::WandererBoots: {
    int seen = 0;
    for (int b = 0; b < BIOME_COUNT; b++)
      seen += level.chart.knows(BiomeId(b));
    return hold(p, &Mob::speed, 0.74f * std::min(25, 5 * seen));
  }
  case Charm::ThunderIdol:
    if (cd > 0)
      return;
    cd = 60;
    return ability::thunder(p, 6, 30);
  case Charm::Hearthstone:
    if (cd > 0 || p.hp > p.maxHp * 0.25f || p.dead())
      return;
    if (const Structure *home = level.night.lastLit()) {
      cd = 90;
      flourish::echo(level, p.x, p.y, p.body(), GOLD);
      p.x = home->centerX(), p.y = home->centerY() + 20;
      p.shield(1.5f);
      level.add<Shockwave>(p.x, p.bodyY(), 4, 50, 0.4f, GOLD);
      Sound::raise.play(0.5f, 0, 1.4f);
    }
    return;
  case Charm::ShrineKey:
    if (level.night.nights != nights)
      nights = level.night.nights, lit = level.night.lit;
    if (level.night.lit == lit)
      level.kindleRate *= 1.5f;
    return;
  case Charm::PackBanner:
    if (level.mob(pet) || cd > 0)
      return;
    cd = 10;
    pet = summon(level, AllyKind::Wolf, p.x - 16, p.y, 12 * p.might, 600, pay::Bleed, this)->id;
    return;
  default:
    return;
  }
}

void WorldRelic::slay(Mob &p, Enemy &e) {
  if (kind == Charm::HollowCrown && e.title) {
    crowns++, p.might += 0.03f;
    Spark::burst(*p.level, p.x, p.bodyY() - 8, GOLD, 8, 60, true);
  }
}

void addWorldRelics(std::vector<std::unique_ptr<Upgrade>> &u) {
  for (int i = 0; i < int(Charm::Count); i++)
    u.push_back(std::make_unique<WorldRelic>(Charm(i)));
}
