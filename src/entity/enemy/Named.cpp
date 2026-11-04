#include "entity/enemy/Named.hpp"

#include <spdlog/fmt/fmt.h>

#include <cctype>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Shockwave.hpp"

namespace {

using K = EnemyKind;
using T = Trait;

std::string shout(const char *name) {
  std::string out = name;
  for (char &c : out)
    c = char(std::toupper(static_cast<unsigned char>(c)));
  return out;
}

} // namespace

const NamedPack NAMED[NAMED_COUNT] = {
    {"Bramblemaw", K::Boar, {T::Hasted, T::Thorned}, K::Thornling, 4, BIOME_MEADOW},
    {"Grizzlegut the Goblin King", K::GoblinShaman, {T::AuraBearer, T::Multishot},
     K::GoblinArcher, 6, BIOME_HEATH},
    {"Mother Mire", K::BogToad, {T::Regenerating, T::Vampiric}, K::BogToad, 5, BIOME_MIRE},
    {"The Drowned Abbot", K::SunkenPriest, {T::Cursed, T::Phasing}, K::Drowned, 6, BIOME_RUINS},
    {"Old Rimefang", K::Yeti, {T::ColdEnchanted, T::StoneSkin}, K::Yeti, 2, BIOME_SNOW},
    {"The Pale Warden", K::BarrowWight, {T::Silencing, T::Blinking}, K::BarrowArcher, 4,
     BIOME_BARROW},
    {"The Red Mesa", K::SaltGolem, {T::FireEnchanted, T::Splitting}, K::SaltGolem, 3,
     BIOME_SALT},
    {"Skyreaver", K::StormKite, {T::StormEnchanted, T::Hasted}, K::StormKite, 5, BIOME_STORM},
    {"Ashmother", K::AshWraith, {T::FireEnchanted, T::Volatile}, K::AshWraith, 6, BIOME_EMBER},
    {"The Hollow King", K::HollowSentinel, {T::StoneSkin, T::AuraBearer}, K::HollowSentinel, 4,
     BIOME_HOLLOW},
};

namespace named {

void tick(Level &level) {
  if ((level.namedScan -= DT) > 0 || (!level.rules.bosses && !level.rules.threat))
    return;
  level.namedScan = 2;
  Player &p = *level.player;
  if (p.dead())
    return;
  BiomeId here = world::biomeUnder(p.x, p.y);
  for (int i = 0; i < NAMED_COUNT; i++) {
    const NamedPack &n = NAMED[i];
    if (n.home != here || level.namedRaised >> i & 1 ||
        level.time < Enemy::typeOf(n.base).unlockAt)
      continue;
    double a = rng.range(0, TAU), r = level.game.screen.reach() + 60;
    if (raise(level, i, p.x + std::cos(a) * r, p.y + std::sin(a) * r))
      level.game.announce(fmt::format("{} STIRS NEARBY", shout(n.name)));
    return;
  }
}

Enemy *raise(Level &level, int index, double x, double y) {
  const NamedPack &n = NAMED[index];
  Enemy *leader = level.spawner.spawn(n.base, x, y, true);
  if (!leader)
    return nullptr;
  level.namedRaised |= 1u << index;
  leader->crown(n.name);
  leader->traits.bits = 0;
  for (Trait t : n.traits)
    leader->traits.add(t);
  leader->pack = &n;
  level.bossIds.push_back(leader->id);
  for (int i = 0; i < n.count; i++) {
    double a = TAU * i / n.count;
    if (Enemy *m = level.spawner.spawn(n.minion, x + std::cos(a) * 30, y + std::sin(a) * 24, true))
      m->leader = leader->id;
  }
  level.add<Shockwave>(x, y, 6, 60, 0.6f, leader->type.blood, false);
  spdlog::info("named: {} with {} of kind {} at {:.0f} s", n.name, n.count, int(n.minion),
               level.time);
  return leader;
}

void fall(Enemy &leader) {
  Level &level = *leader.level;
  level.game.announce(fmt::format("{} HAS FALLEN", shout(leader.pack->name)));
  if (!level.rules.loot)
    return;
  Player &p = *level.player;
  int mlvl = std::clamp(level.monsterLevel(leader.x, leader.y) + 4, 1, 99);
  Item rare = rollItem(mlvl, p.bonus().magicFind, !level.night.dawn, rng, Rarity::Rare);
  double a = rng.range(0, TAU);
  level.add<ItemDrop>(leader.x, leader.y, Drop{DropKind::Item, 1, rare},
                      leader.x + std::cos(a) * 20, leader.y + std::sin(a) * 14);
}

} // namespace named
