#include "level/BiomeEvents.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "gfx/EnemyWarning.hpp"
#include "level/Level.hpp"
#include "level/Noise.h"
#include "particle/Spark.hpp"

namespace {
const char *const NAMES[] = {
    "SALT CRUST FRACTURES", "FLOOD CURRENT RISES", "MOONLIGHT PURIFIES",
    "THE RUIN WARD AWAKENS", "THE FEN FLARES", "AMBER SAP SEEPS",
    "THE STORM CHARGES", "GLASS SHARDS ERUPT", "HOLLOW SPORES BLOOM",
    "A STAR WILL FALL"};

const Color TONES[] = {
    {0.95f, 0.9f, 0.7f, 1}, {0.3f, 0.65f, 1, 1},
    {0.7f, 0.7f, 1, 1}, {0.6f, 0.9f, 0.85f, 1},
    {1, 0.4f, 0.15f, 1}, {1, 0.7f, 0.2f, 1},
    {0.5f, 0.8f, 1, 1}, {0.9f, 0.65f, 1, 1},
    {0.7f, 0.9f, 0.3f, 1}, {1, 0.85f, 0.5f, 1}};

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
  if (biome < BIOME_SALT || biome > BIOME_STARFALL || epoch < 0)
    return {};
  uint32_t h = mix(seed ^ uint32_t(biome) * 0x9E3779B1u ^
                   uint32_t(epoch) * 0x85EBCA6Bu ^ uint32_t(rx) * 0xC2B2AE35u ^
                   uint32_t(ry) * 0x27D4EB2Fu);
  BiomeEventSchedule e;
  e.biome = biome;
  e.epoch = epoch;
  e.begins = epoch * 28.0f + 8 + float(h % 11);
  e.ends = e.begins + 5;
  e.radius = biome == BIOME_STARFALL ? 42.0f : 64.0f;
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
  int epoch = int(level.time / 28);
  if (!active && biome >= BIOME_SALT && biome <= BIOME_STARFALL &&
      handled[biome - BIOME_SALT] != epoch) {
    auto candidate = schedule(noise_salt, biome, epoch, world::regionOf(tx),
                              world::regionOf(ty));
    if (level.time >= candidate.begins) {
      handled[biome - BIOME_SALT] = epoch;
    } else if (level.time >= candidate.begins - 3) {
      handled[biome - BIOME_SALT] = epoch;
      if (level.time <= candidate.begins - 3 + DT * 1.5f &&
          !sanctuary(p.x, p.y) && !sanctuary(candidate.x, candidate.y)) {
        current = candidate;
        active = true;
        nextPulse = current.begins;
        level.game.announce(std::string(NAMES[biome - BIOME_SALT]) + " - MOVE TO SAFETY");
      }
    }
  }
  if (!active || level.time < current.begins)
    return;
  double xd = p.x - current.x, yd = p.y - current.y;
  bool inside = xd * xd + yd * yd < current.radius * current.radius &&
                !sanctuary(p.x, p.y);
  if (current.biome == BIOME_FLOOD) {
    if (inside) {
      p.xa += current.dx * 95 * DT;
      p.ya += current.dy * 95 * DT;
    }
    level.enemiesNear(current.x, current.y, current.radius, [&](Enemy &e) {
      double dx = e.x - current.x, dy = e.y - current.y;
      if (dx * dx + dy * dy < current.radius * current.radius) {
        e.xa += current.dx * 55 * DT;
        e.ya += current.dy * 55 * DT;
      }
    });
  }
  if (inside && current.biome == BIOME_STORM)
    p.skillTime = std::max(0.0f, p.skillTime - DT * 2);
  if (level.time < nextPulse)
    return;
  bool first = nextPulse == current.begins;
  nextPulse = level.time + 0.5f;
  if (inside) {
    if (first) level.handicaps++;
    p.felledBy = NAMES[current.biome - BIOME_SALT];
    switch (current.biome) {
    case BIOME_SALT:
      if (first)
        p.hurt(8);
      break;
    case BIOME_MOON:
      p.status.effects[int(StatusKind::Burn)] = {};
      p.status.effects[int(StatusKind::Poison)] = {};
      break;
    case BIOME_RUINS:
      p.afflict(StatusKind::Marked, 0.7f, 0.15f);
      break;
    case BIOME_EMBER:
      p.afflict(StatusKind::Burn, 0.7f, 3);
      break;
    case BIOME_AMBER:
      p.afflict(StatusKind::Chill, 0.7f, 0.3f);
      break;
    case BIOME_GLASS:
      p.hurt(4);
      break;
    case BIOME_HOLLOW:
      p.afflict(StatusKind::Poison, 0.7f, 2.5f);
      break;
    case BIOME_STARFALL:
      if (first)
        p.hurt(14);
      break;
    default:
      break;
    }
  }
  level.enemiesNear(current.x, current.y, current.radius + 24, [&](Enemy &e) {
    double dx = e.x - current.x, dy = e.y - current.y;
    if (dx * dx + dy * dy > current.radius * current.radius ||
        sanctuary(e.x, e.y))
      return;
    switch (current.biome) {
    case BIOME_SALT:
      e.afflict(StatusKind::Chill, 0.7f, 0.4f);
      break;
    case BIOME_MOON:
    case BIOME_RUINS:
    case BIOME_STORM:
      e.afflict(StatusKind::Marked, 0.7f, 0.3f);
      break;
    case BIOME_EMBER:
      e.afflict(StatusKind::Burn, 0.7f, 6);
      break;
    case BIOME_AMBER:
    case BIOME_GLASS:
      e.afflict(StatusKind::Chill, 0.7f, 0.6f);
      break;
    case BIOME_HOLLOW:
      e.afflict(StatusKind::Poison, 0.7f, 4);
      break;
    case BIOME_STARFALL:
      if (first)
        e.hurt(45, dx, dy, 100);
      break;
    default:
      break;
    }
  });
  if (Spark::alive < 450)
    Spark::burst(level, current.x, current.y - 3,
                 TONES[current.biome - BIOME_SALT], 8, 55, true);
}

void BiomeEvents::render(Screen &s, const Level &level) const {
  if (!active || current.biome < BIOME_SALT ||
      !s.visible(current.x, current.y, current.radius + 20))
    return;
  Color c = TONES[current.biome - BIOME_SALT];
  bool warning = level.time < current.begins;
  bool dangerous = current.biome != BIOME_MOON && current.biome != BIOME_STORM;
  if (dangerous)
    enemyWarning(s, current.x, current.y, current.radius,
                 warning ? 0.8f : 0.5f, level.time * 0.15f);
  else {
    s.color(c.alpha(warning ? 0.8f : 0.4f));
    s.center(atlas::Ring, current.x, current.y,
             current.radius * 2 / atlas::Ring.w, level.time * 0.15f);
  }
  if (warning) {
    float progress = clampf((level.time - current.begins + 3) / 3, 0, 1);
    s.color((dangerous ? col::red : c).alpha(0.7f));
    s.center(atlas::Ring, current.x, current.y,
             current.radius * 2 * progress / atlas::Ring.w);
  } else {
    s.color(c.alpha(0.16f));
    s.center(atlas::Glow, current.x, current.y,
             current.radius * 2 / atlas::Glow.w);
  }
  s.color(col::white);
}
