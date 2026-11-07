#include "hero/Mutation.hpp"

#include <cmath>

#include "entity/Ability.hpp"
#include "entity/Faction.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Spark.hpp"

namespace {

using M = Mutation;

constexpr Color EMBER{1, 0.5f, 0.15f, 1}, RIME{0.7f, 0.88f, 1, 1};

struct Whisper {
  BiomeId biome;
  Mutation gift;
};

constexpr Whisper WHISPERS[] = {
    {BIOME_FUNGAL, M::SporeSacs}, {BIOME_GLASS, M::Chitin},       {BIOME_CRYSTAL, M::Quills},
    {BIOME_ASHLANDS, M::EmberGlands}, {BIOME_SNOW, M::RimeGlands}, {BIOME_MIRE, M::Gills},
    {BIOME_MOON, M::Stillness},   {BIOME_STORM, M::Stormcall},   {BIOME_EMBER, M::Kindle},
    {BIOME_STARFALL, M::HollowBones}, {BIOME_SAVANNA, M::Sunskin},
};

struct Surge {
  CaveBiome biome;
  Mutation a, b;
};

constexpr Surge SURGES[] = {
    {CaveBiome::Glowcap, M::SporeSacs, M::Stillness}, {CaveBiome::Mere, M::Gills, M::Stormcall},
    {CaveBiome::Magma, M::EmberGlands, M::Kindle},    {CaveBiome::Cathedral, M::Chitin, M::Quills},
    {CaveBiome::Ossuary, M::HollowBones, M::Quills},  {CaveBiome::Abyss, M::Stormcall, M::Stillness},
    {CaveBiome::Hive, M::Chitin, M::SporeSacs},
};

bool smothered(Player &p) {
  bool smoke = false;
  p.level->enemiesNear(p.x, p.y, mutation::SMOTHER, [&](Enemy &e) { smoke |= e.is(EnemyKind::Censer); });
  return smoke;
}

void witnessed(Player &p) {
  bool seen = false;
  p.level->enemiesNear(p.x, p.y, mutation::WITNESS, [&](Enemy &e) { seen |= factionOf(e) == Faction::Synod; });
  if (seen)
    p.level->suspect(mutation::CAST_SCRUTINY);
}

void kindle(Player &p) {
  std::vector<Enemy *> near = p.level->nearest(p.x, p.y, 200, 1);
  if (near.empty())
    return;
  Enemy &e = *near.front();
  int tx = world::tileOf(e.x), ty = world::tileOf(e.y - 2);
  if (p.level->rules.matter)
    for (int dy = -1; dy <= 1; dy++)
      for (int dx = -1; dx <= 1; dx++)
        p.level->matter.set(tx + dx, ty + dy, Stuff::Burning, 4);
  e.afflict(StatusKind::Burn, 3, 10 * p.power());
  Spark::burst(*p.level, e.x, e.y - 6, EMBER, 8, 50, false);
}

void stillness(Player &p) {
  p.level->enemiesNear(p.x, p.y, 70, [&](Enemy &e) { e.afflict(StatusKind::Chill, 2, 1); });
  int tx = world::tileOf(p.x), ty = world::tileOf(p.y - 2);
  for (int dy = -2; dy <= 2; dy++)
    for (int dx = -2; dx <= 2; dx++)
      if (dx * dx + dy * dy <= 5)
        p.level->react(tx + dx, ty + dy, Cause::Chill);
  Spark::burst(*p.level, p.x, p.y - 6, RIME, 10, 60, false);
}

} // namespace

const MutationDef MUTATIONS[int(Mutation::Count)] = {
    {"CHITIN", false, false, 0},     {"HOLLOW BONES", false, false, 0}, {"QUILLS", false, false, 0},
    {"EMBER GLANDS", false, false, 0.6f}, {"RIME GLANDS", false, false, 0.6f}, {"GILLS", false, false, 0},
    {"SUNSKIN", false, false, 0},    {"SPORE SACS", false, false, 0},  {"KINDLE", true, false, 6},
    {"STILLNESS", true, false, 8},   {"STORMCALL", true, false, 7},    {"BRITTLE", false, true, 0.5f},
    {"SPOREBLOOM", false, true, 25}, {"FRAIL", false, true, 0},        {"SLUGGISH", false, true, 0},
};

namespace mutation {

bool has(const Player &p, Mutation m) { return p.mutations >> int(m) & 1; }

void grant(Player &p, Mutation m) {
  if (m == Mutation::Count || has(p, m))
    return;
  p.mutations |= 1u << int(m);
  p.mutationTime[int(m)] = MUTATIONS[int(m)].every;
  switch (m) {
  case M::Chitin:
    p.armor += 4, p.speed *= 0.92f;
    break;
  case M::HollowBones:
    p.speed *= 1.12f, p.maxHp *= 0.85f, p.hp = std::min(p.hp, p.maxHp);
    break;
  case M::Quills:
    p.traits.add(Trait::Thorned);
    break;
  case M::EmberGlands:
    p.status.resist[int(StatusKind::Burn)] += 0.5f;
    break;
  case M::SporeSacs:
    p.status.immune |= 1u << int(StatusKind::Poison);
    break;
  case M::Frail:
    p.maxHp *= 0.9f, p.hp = std::min(p.hp, p.maxHp);
    break;
  case M::Sluggish:
    p.speed *= 0.92f;
    break;
  default:
    break;
  }
}

void tick(Player &p) {
  if (!p.mutations || p.dead())
    return;
  Level &l = *p.level;
  if (has(p, M::Sunskin) && l.night.dawn)
    p.mend(1.5f * DT);
  for (int i = 0; i < int(Mutation::Count); i++) {
    const MutationDef &d = MUTATIONS[i];
    if (!(p.mutations >> i & 1) || d.every <= 0 || (p.mutationTime[i] -= DT) > 0)
      continue;
    p.mutationTime[i] = d.every;
    if (d.mind && smothered(p))
      continue;
    switch (Mutation(i)) {
    case M::EmberGlands:
      if (p.status.moving && l.rules.matter)
        l.matter.set(world::tileOf(p.x - p.xa * 0.2), world::tileOf(p.y - 2 - p.ya * 0.2), Stuff::Burning, 2);
      break;
    case M::RimeGlands:
      l.enemiesNear(p.x, p.y, 30, [&](Enemy &e) { e.afflict(StatusKind::Chill, 1.5f, 1); });
      break;
    case M::Kindle:
      kindle(p);
      break;
    case M::Stillness:
      stillness(p);
      break;
    case M::Stormcall:
      ability::chain(p, 3, 160, 14);
      break;
    case M::Brittle:
      if (l.stuffAt(world::tileOf(p.x), world::tileOf(p.y - 2)) == Stuff::Frozen)
        p.status.apply(StatusKind::Burn, 0.5f, 4);
      break;
    case M::Sporebloom:
      for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
          l.react(world::tileOf(p.x) + dx, world::tileOf(p.y - 2) + dy, Cause::Spore);
      break;
    default:
      break;
    }
    if (d.mind)
      witnessed(p);
  }
}

Mutation whisper(BiomeId biome) {
  for (const Whisper &w : WHISPERS)
    if (w.biome == biome)
      return w.gift;
  return Mutation::Count;
}

void surge(CaveBiome biome, Mutation &a, Mutation &b) {
  a = b = Mutation::Count;
  for (const Surge &s : SURGES)
    if (s.biome == biome)
      a = s.a, b = s.b;
}

Mutation random(uint32_t roll, bool defect, uint32_t owned) {
  int pool[int(Mutation::Count)], n = 0;
  for (int i = 0; i < int(Mutation::Count); i++)
    if (MUTATIONS[i].defect == defect && !(owned >> i & 1))
      pool[n++] = i;
  return n ? Mutation(pool[roll % uint32_t(n)]) : Mutation::Count;
}

uint32_t starting(uint32_t seed) {
  uint32_t owned = 0;
  for (int k = 0; k < 3; k++) {
    Mutation m = random(hash2(int(seed) + k * 977, k * 131 + 7), k == 2, owned);
    if (m != Mutation::Count && !MUTATIONS[int(m)].mind)
      owned |= 1u << int(m);
    else if (k < 2)
      for (int i = 0; i < int(Mutation::Count); i++)
        if (!MUTATIONS[i].mind && !MUTATIONS[i].defect && !(owned >> i & 1)) {
          owned |= 1u << i;
          break;
        }
  }
  return owned;
}

} // namespace mutation
