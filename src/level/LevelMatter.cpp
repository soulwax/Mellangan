#include "level/Level.hpp"

#include <cstring>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Edits.hpp"
#include "level/Prop.hpp"
#include "level/World.hpp"
#include "particle/Spark.hpp"
#include "Math.hpp"
#include "entity/Faction.hpp"

namespace {

enum class Effect : uint8_t { None, Steam, Burst, Current };

struct Reaction {
  Stuff from;
  Cause cause;
  Stuff to;
  float seconds;
  bool fuel;
  Effect effect;
};

constexpr Reaction REACTIONS[] = {
    {Stuff::None, Cause::Fire, Stuff::Burning, Level::BURN, true, Effect::None},
    {Stuff::None, Cause::Water, Stuff::Wet, 8, false, Effect::None},
    {Stuff::None, Cause::Spore, Stuff::Spored, 10, false, Effect::None},
    {Stuff::Wet, Cause::Chill, Stuff::Frozen, 12, false, Effect::None},
    {Stuff::Wet, Cause::Fire, Stuff::None, 0, false, Effect::Steam},
    {Stuff::Wet, Cause::Charge, Stuff::Charged, 3, false, Effect::Current},
    {Stuff::Frozen, Cause::Fire, Stuff::Wet, 8, false, Effect::Steam},
    {Stuff::Burning, Cause::Chill, Stuff::Ash, 20, false, Effect::Steam},
    {Stuff::Burning, Cause::Water, Stuff::Wet, 6, false, Effect::Steam},
    {Stuff::Ash, Cause::Water, Stuff::Wet, 8, false, Effect::None},
    {Stuff::Oiled, Cause::Fire, Stuff::Burning, 6, false, Effect::Burst},
    {Stuff::Spored, Cause::Fire, Stuff::Burning, 3, false, Effect::None},
    {Stuff::Spored, Cause::Spore, Stuff::Spored, 10, false, Effect::None},
};

constexpr Color LOOK_SPORE{0.62f, 0.4f, 0.78f, 1};
constexpr Color STEAM{0.9f, 0.92f, 0.95f, 0.8f}, FLARE{1, 0.55f, 0.15f, 1}, SPARK{0.6f, 0.8f, 1, 1};

constexpr const char *KINDLING[] = {"rass", "Tuft", "bush", "Bush", "heather", "reeds", "bracken", "fern", "Fern",
                                    "oak",  "Oak",  "pine", "Pine", "birch",   "Birch", "ree",     "log",  "Log",
                                    "stump", "willow", "Willow", "shroom", "ushroom", "toadstool", "cattails", "ivy"};

bool mycel(Mob &m) {
  auto *e = dynamic_cast<Enemy *>(&m);
  return e && factionOf(*e) == Faction::Mycel;
}

bool kindling(int x, int y) {
  Prop p;
  if (world::isWater(x, y) || !world::propAt(x, y, p) || !p.kind)
    return false;
  for (const char *k : KINDLING)
    if (std::strstr(p.kind, k))
      return true;
  return false;
}

} // namespace

Stuff Level::stuffAt(int x, int y) const {
  if (Stuff s = matter.at(x, y); s != Stuff::None)
    return s;
  if (world::isLava(x, y))
    return Stuff::Burning;
  if (world::layer() == world::Layer::Surface && world::biomeAt(x, y) == BIOME_BADLANDS &&
      hash2(x / 2 * 7 + 3, y / 2 * 11 - 5) % TAR == 0 && !world::isWater(x, y))
    return Stuff::Oiled;
  return sky.puddle(x, y) > PUDDLE ? Stuff::Wet : Stuff::None;
}

bool Level::react(int x, int y, Cause c) {
  if (!rules.matter)
    return false;
  if (!matter.fuel)
    matter.fuel = kindling;
  Stuff from = stuffAt(x, y);
  for (const Reaction &r : REACTIONS) {
    if (r.from != from || r.cause != c || (r.fuel && !matter.fuel(x, y)))
      continue;
    matter.set(x, y, r.to, r.seconds);
    double cx = x * 16 + 8.0, cy = y * 16 + 8.0;
    if ((r.to == Stuff::Burning || r.to == Stuff::Spored) &&
        std::hypot(cx - player->x, cy - player->y) < CHAOS_NEAR)
      suspect(SUSPECT_CHAOS);
    if (r.effect == Effect::Steam)
      Spark::burst(*this, cx, cy, STEAM, 4, 25, false);
    if (r.effect == Effect::Burst) {
      Spark::burst(*this, cx, cy, FLARE, 10, 70, false);
      for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
          if ((dx || dy) && matter.at(x + dx, y + dy) != Stuff::Limed)
            matter.set(x + dx, y + dy, Stuff::Burning, 3);
    }
    if (r.effect == Effect::Current)
      current(x, y);
    return true;
  }
  return false;
}

void Level::current(int x, int y) {
  std::vector<std::pair<int, int>> queue{{x, y}};
  for (size_t q = 0; q < queue.size() && queue.size() < CURRENT; q++) {
    constexpr int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto &d : D) {
      int nx = queue[q].first + d[0], ny = queue[q].second + d[1];
      if (stuffAt(nx, ny) != Stuff::Wet || queue.size() >= CURRENT)
        continue;
      matter.set(nx, ny, Stuff::Charged, 3);
      queue.push_back({nx, ny});
    }
  }
  Spark::burst(*this, x * 16 + 8.0, y * 16 + 8.0, SPARK, 6, 60, false);
}

void Level::sprout(double x, double y) {
  if (int(sprouts.size()) < SPROUTS && stuffAt(world::tileOf(x), world::tileOf(y - 2)) == Stuff::Spored)
    sprouts.push_back({x, y, time + SPROUT_DELAY});
}

void Level::scorch(int x, int y) {
  if (world::edits && !world::edits->at(x, y) && matter.fuel && matter.fuel(x, y))
    world::edits->harvest(x, y, float(night.nights + SCORCH_DAYS));
}

float Level::footing(double x, double y) const {
  if (matter.empty())
    return 1;
  Stuff s = matter.at(world::tileOf(x), world::tileOf(y - 2));
  return s == Stuff::Wet ? WET_PACE : s == Stuff::Frozen ? ICE_PACE : s == Stuff::Oiled ? OIL_PACE : 1;
}

void Level::tread(Mob &m, bool boss) {
  Stuff s = stuffAt(world::tileOf(m.x), world::tileOf(m.y - 2));
  if (s == Stuff::Burning) {
    float power = m.asPlayer() ? game.balance.enemyDamage * HEAT : HEAT * 1.5f;
    m.afflict(StatusKind::Burn, 1.0f, boss ? power / 2 : power);
  } else if (s == Stuff::Wet && m.status.effect(StatusKind::Burn).time > 0) {
    m.status.end(StatusKind::Burn);
  } else if (s == Stuff::Charged) {
    float power = m.asPlayer() ? game.balance.enemyDamage * HEAT * 2 : HEAT * 3;
    m.status.apply(StatusKind::Burn, 0.5f, boss ? power / 2 : power);
  } else if (s == Stuff::Spored && !mycel(m)) {
    m.status.apply(StatusKind::Poison, 1.0f, m.asPlayer() ? game.balance.enemyDamage * 2 : 3);
  } else if (s == Stuff::Limed) {
    m.status.end(StatusKind::Burn), m.status.end(StatusKind::Poison);
  }
}

void Level::matterTick() {
  if (!rules.matter)
    return;
  matter.tick(*this);
  if (!player->dead())
    tread(*player, false);
  for (size_t i = 0; i < sprouts.size();) {
    if (sprouts[i].due > time) {
      i++;
      continue;
    }
    if (Enemy *e = spawner.spawn(EnemyKind::Sporeling, sprouts[i].x, sprouts[i].y, true))
      Spark::burst(*this, e->x, e->y - 4, LOOK_SPORE, 6, 40, false);
    sprouts.erase(sprouts.begin() + long(i));
  }
}
