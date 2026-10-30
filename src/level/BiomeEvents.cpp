#include "level/BiomeEvents.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "gfx/EnemyWarning.hpp"
#include "level/Level.hpp"
#include "level/Noise.h"
#include "particle/Drift.hpp"
#include "particle/Spark.hpp"

namespace {
namespace pt = atlas::pt;
using S = StatusKind;
using K = EnemyKind;
constexpr S NONE = S::Count;

struct EventDef {
  const char *name;
  Color tone;
  int icon;
  float radius = 64, every = 48;
  bool boon = false, cleanse = false, charge = false;
  float strike = 0, sting = 0, heal = 0, push = 0, repel = 0;
  S status = NONE;
  float power = 0;
  float foeStrike = 0;
  S foeStatus = NONE;
  float foePower = 0;
  K raise = K::Count;
  int count = 0;
};

const EventDef EVENTS[BIOME_COUNT] = {
    {.name = "A WARM BREEZE", .tone = {1, 0.9f, 0.55f, 1},
     .icon = pt::Butterfly, .boon = true, .heal = 1.5f},
    {.name = "THE OLD OAKS SHELTER", .tone = {0.55f, 0.9f, 0.45f, 1},
     .icon = pt::Leaf, .boon = true, .repel = 150},
    {.name = "A HARVEST GALE", .tone = {1, 0.6f, 0.25f, 1},
     .icon = pt::MapleLeaf, .push = 120},
    {.name = "WILDFIRE SPREADS", .tone = {1, 0.45f, 0.15f, 1},
     .icon = pt::Ember, .status = S::Burn, .power = 3,
     .foeStatus = S::Burn, .foePower = 6},
    {.name = "THE BARROW OPENS", .tone = {0.75f, 0.85f, 0.7f, 1},
     .icon = pt::Bone, .raise = K::Skeleton, .count = 5},
    {.name = "A BLIZZARD HOWLS", .tone = {0.8f, 0.92f, 1, 1},
     .icon = pt::Snowflake, .push = 50, .status = S::Chill, .power = 0.35f,
     .foeStatus = S::Chill, .foePower = 0.6f},
    {.name = "PETALS FALL", .tone = {1, 0.7f, 0.85f, 1}, .icon = pt::Petal,
     .boon = true, .cleanse = true, .heal = 1},
    {.name = "BOG GAS RISES", .tone = {0.65f, 0.9f, 0.35f, 1},
     .icon = pt::Bubble, .status = S::Poison, .power = 2,
     .foeStatus = S::Poison, .foePower = 4},
    {.name = "A SANDSTORM SWEEPS", .tone = {1, 0.82f, 0.5f, 1}, .icon = 9,
     .sting = 2, .push = 80, .foeStatus = S::Chill, .foePower = 0.4f},
    {.name = "THE VINES GRASP", .tone = {0.4f, 0.85f, 0.35f, 1},
     .icon = pt::Leaf, .status = S::Chill, .power = 0.5f,
     .foeStatus = S::Chill, .foePower = 0.8f},
    {.name = "A STAMPEDE THUNDERS", .tone = {0.95f, 0.8f, 0.45f, 1},
     .icon = pt::Puff, .strike = 10, .push = 140, .foeStrike = 30},
    {.name = "THE GROUND ERUPTS", .tone = {1, 0.4f, 0.12f, 1},
     .icon = pt::Rock, .radius = 48, .strike = 16, .status = S::Burn,
     .power = 3, .foeStrike = 50},
    {.name = "THE CRYSTALS RESONATE", .tone = {0.8f, 0.7f, 1, 1},
     .icon = pt::ArcaneStar, .boon = true, .charge = true,
     .foeStatus = S::Chill, .foePower = 0.5f},
    {.name = "A SPORE BLOOM", .tone = {0.95f, 0.65f, 0.9f, 1},
     .icon = pt::Spore, .boon = true, .heal = 1.5f, .foeStatus = S::Poison,
     .foePower = 4},
    {.name = "ROCKS FALL", .tone = {0.85f, 0.55f, 0.35f, 1}, .icon = pt::Rock,
     .radius = 48, .strike = 12, .foeStrike = 40},
    {.name = "WOLVES ENCIRCLE", .tone = {0.75f, 0.85f, 0.95f, 1},
     .icon = pt::Snowflake, .raise = K::Wolf, .count = 4},
    {.name = "SALT CRUST FRACTURES", .tone = {0.95f, 0.9f, 0.7f, 1},
     .icon = pt::FrostGlint, .every = 28, .strike = 8,
     .foeStatus = S::Chill, .foePower = 0.4f},
    {.name = "FLOOD CURRENT RISES", .tone = {0.3f, 0.65f, 1, 1},
     .icon = pt::Rain, .every = 28, .push = 95},
    {.name = "MOONLIGHT PURIFIES", .tone = {0.7f, 0.7f, 1, 1},
     .icon = pt::HolyGlint, .every = 28, .boon = true, .cleanse = true,
     .foeStatus = S::Marked, .foePower = 0.3f},
    {.name = "THE RUIN WARD AWAKENS", .tone = {0.6f, 0.9f, 0.85f, 1},
     .icon = pt::Rune, .every = 28, .status = S::Marked, .power = 0.15f,
     .foeStatus = S::Marked, .foePower = 0.3f},
    {.name = "THE FEN FLARES", .tone = {1, 0.4f, 0.15f, 1}, .icon = pt::Ember,
     .every = 28, .status = S::Burn, .power = 3, .foeStatus = S::Burn,
     .foePower = 6},
    {.name = "AMBER SAP SEEPS", .tone = {1, 0.7f, 0.2f, 1}, .icon = pt::Wisp,
     .every = 28, .status = S::Chill, .power = 0.3f, .foeStatus = S::Chill,
     .foePower = 0.6f},
    {.name = "THE STORM CHARGES", .tone = {0.5f, 0.8f, 1, 1},
     .icon = pt::ArcaneStar, .every = 28, .boon = true, .charge = true,
     .foeStatus = S::Marked, .foePower = 0.3f},
    {.name = "GLASS SHARDS ERUPT", .tone = {0.9f, 0.65f, 1, 1},
     .icon = pt::IceShard, .every = 28, .sting = 4, .foeStatus = S::Chill,
     .foePower = 0.6f},
    {.name = "HOLLOW SPORES BLOOM", .tone = {0.7f, 0.9f, 0.3f, 1},
     .icon = pt::Spore, .every = 28, .status = S::Poison, .power = 2.5f,
     .foeStatus = S::Poison, .foePower = 4},
    {.name = "A STAR WILL FALL", .tone = {1, 0.85f, 0.5f, 1},
     .icon = pt::FallingStar, .radius = 42, .every = 28, .strike = 14,
     .foeStrike = 45}};

uint32_t mix(uint32_t n) {
  n ^= n >> 16;
  n *= 0x7FEB352Du;
  n ^= n >> 15;
  n *= 0x846CA68Bu;
  return n ^ n >> 16;
}

bool sanctuary(double x, double y) {
  Structure site;
  return world::structureAt(int(std::floor(x / 16)),
                            int(std::floor(y / 16)), site, 2) &&
         site.kind == StructureKind::Shrine;
}
}

BiomeEventSchedule BiomeEvents::schedule(uint32_t seed, int biome, int epoch,
                                         int rx, int ry) {
  if (biome < 0 || biome >= BIOME_COUNT || epoch < 0)
    return {};
  const EventDef &def = EVENTS[biome];
  uint32_t h = mix(seed ^ uint32_t(biome) * 0x9E3779B1u ^
                   uint32_t(epoch) * 0x85EBCA6Bu ^ uint32_t(rx) * 0xC2B2AE35u ^
                   uint32_t(ry) * 0x27D4EB2Fu);
  BiomeEventSchedule e;
  e.biome = biome;
  e.epoch = epoch;
  e.begins = epoch * def.every + 8 + float(h % uint32_t(def.every - 17));
  e.ends = e.begins + 5;
  e.radius = def.radius;
  e.x = (rx * world::REGION + world::REGION / 2) * 16.0 +
        int(h >> 8 & 63) - 31;
  e.y = (ry * world::REGION + world::REGION / 2) * 16.0 +
        int(h >> 16 & 63) - 31;
  double angle = (h >> 24) * TAU / 256;
  e.dx = std::cos(angle);
  e.dy = std::sin(angle);
  return e;
}

void BiomeEvents::tick(Level &level) {
  Player &p = *level.player;
  if (p.dead() || level.night.dawn) {
    active = false;
    return;
  }
  if (active && level.time >= current.ends)
    active = false;
  int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16));
  int biome = world::biomeAt(tx, ty);
  int epoch = int(level.time / EVENTS[biome].every);
  if (!active && level.time >= 40 && handled[biome] != epoch) {
    auto candidate = schedule(noise_salt, biome, epoch, world::regionOf(tx),
                              world::regionOf(ty));
    if (level.time >= candidate.begins) {
      handled[biome] = epoch;
    } else if (level.time >= candidate.begins - 3) {
      handled[biome] = epoch;
      if (level.time <= candidate.begins - 3 + DT * 1.5f &&
          !sanctuary(p.x, p.y) && !sanctuary(candidate.x, candidate.y)) {
        current = candidate;
        active = true;
        nextPulse = current.begins;
        level.game.announce(std::string(EVENTS[biome].name) +
                            (EVENTS[biome].boon ? " - STEP INSIDE"
                                                : " - MOVE TO SAFETY"));
      }
    }
  }
  if (!active || level.time < current.begins)
    return;
  const EventDef &def = EVENTS[current.biome];
  double r2 = current.radius * current.radius;
  double xd = p.x - current.x, yd = p.y - current.y;
  bool inside = xd * xd + yd * yd < r2 && !sanctuary(p.x, p.y);
  if (inside && def.push > 0) {
    p.xa += current.dx * def.push * DT;
    p.ya += current.dy * def.push * DT;
  }
  if (inside && def.charge)
    p.skillTime = std::max(0.0f, p.skillTime - DT * 2);
  if (def.push > 0 || def.repel > 0)
    level.enemiesNear(current.x, current.y, current.radius, [&](Enemy &e) {
      double dx = e.x - current.x, dy = e.y - current.y, d2 = dx * dx + dy * dy;
      if (d2 >= r2)
        return;
      double d = std::max(1.0, std::sqrt(d2));
      e.xa += (current.dx * def.push * 0.58 + dx / d * def.repel) * DT;
      e.ya += (current.dy * def.push * 0.58 + dy / d * def.repel) * DT;
    });
  if (level.time < nextPulse)
    return;
  bool first = nextPulse == current.begins;
  nextPulse = level.time + 0.5f;
  if (first)
    for (int i = 0; i < def.count; i++) {
      double a = TAU * i / def.count + rng.range(-0.3f, 0.3f),
             d = current.radius * rng.range(0.4f, 0.8f);
      double sx = current.x + std::cos(a) * d, sy = current.y + std::sin(a) * d;
      level.spawner.spawn(def.raise, sx, sy, true);
      Spark::burst(level, sx, sy - 4, def.tone, 6, 40, false);
    }
  if (inside) {
    if (!def.boon) {
      if (first)
        level.handicaps++;
      p.felledBy = def.name;
    }
    if (def.cleanse) {
      p.status.effects[int(S::Burn)] = {};
      p.status.effects[int(S::Poison)] = {};
    }
    if (def.heal > 0)
      p.heal(def.heal);
    if (first && def.strike > 0)
      p.hurt(def.strike);
    if (def.sting > 0)
      p.hurt(def.sting);
    if (def.status != NONE)
      p.afflict(def.status, 0.7f, def.power);
  }
  level.enemiesNear(current.x, current.y, current.radius + 24, [&](Enemy &e) {
    double dx = e.x - current.x, dy = e.y - current.y;
    if (dx * dx + dy * dy > r2 || sanctuary(e.x, e.y))
      return;
    if (def.foeStatus != NONE)
      e.afflict(def.foeStatus, 0.7f, def.foePower);
    if (first && def.foeStrike > 0)
      e.hurt(def.foeStrike, dx, dy, 100);
  });
  if (Spark::alive < 450)
    Spark::burst(level, current.x, current.y - 3, def.tone, 8, 55, true);
  for (int i = 0; i < 3 && Drift::room(); i++) {
    double a = rng.range(0, TAU), d = current.radius * std::sqrt(rng.unit());
    Drift *f = level.add<Drift>(current.x + std::cos(a) * d,
                                current.y + std::sin(a) * d,
                                current.dx * def.push * 0.5, -6, def.icon,
                                def.tone.alpha(0.85f), 1.2f);
    f->sway = 8, f->scale = 0.7f;
  }
}

void BiomeEvents::render(Screen &s, const Level &level) const {
  if (!active || current.biome < 0 ||
      !s.visible(current.x, current.y, current.radius + 20))
    return;
  const EventDef &def = EVENTS[current.biome];
  Color c = def.tone;
  bool warning = level.time < current.begins;
  if (!def.boon)
    enemyWarning(s, current.x, current.y, current.radius,
                 warning ? 0.8f : 0.5f, level.time * 0.15f);
  else {
    s.color(c.alpha(warning ? 0.8f : 0.4f));
    s.ring(current.x, current.y, current.radius);
  }
  if (warning) {
    float progress = clampf((level.time - current.begins + 3) / 3, 0, 1);
    s.color((def.boon ? c : col::red).alpha(0.7f));
    s.ring(current.x, current.y, current.radius * progress);
  } else {
    s.color(c.alpha(0.16f));
    s.center(atlas::Glow, current.x, current.y,
             current.radius * 2 / atlas::Glow.w);
  }
  s.color(col::white);
}
