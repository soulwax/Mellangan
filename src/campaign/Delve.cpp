#include "campaign/Delve.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

using K = EnemyKind;

struct Wave {
  K kind;
  int count;
};

const Wave WAVES[Delve::FLOORS][3] = {
    {{K::CaveBat, 2}, {K::KoboldMiner, 5}, {K::Count, 0}},
    {{K::RockCrawler, 4}, {K::OreGolem, 2}, {K::CaveBat, 1}},
    {{K::Count, 1}, {K::KoboldMiner, 4}, {K::RockCrawler, 2}},
};

constexpr Color DUST{0.6f, 0.55f, 0.5f, 1};

} // namespace

EnemyKind Delve::guardian(int act) {
  constexpr K BOSSES[] = {K::Cyclops, K::MycelialBrute, K::Iceback, K::GlassMantis, K::Cyclops};
  return BOSSES[std::clamp(act, 1, 5) - 1];
}

bool Delve::entrance(const Level &level, const Campaign &c, Structure &out) {
  const Player &p = *level.player;
  if (!world::structureAt(int(std::floor(p.x / 16)), int(std::floor(p.y / 16)), out, 3) ||
      out.kind != StructureKind::Mine)
    return false;
  if (c.delved(out.rx, out.ry))
    return false;
  double xd = out.centerX() - p.x, yd = out.centerY() - p.y;
  return xd * xd + yd * yd < REACH * REACH;
}

void Delve::seal(Campaign &c) {
  int cx = int(std::floor(x / 16)), cy = int(std::floor(y / 16));
  for (int ty = cy - RING; ty <= cy + RING; ty++)
    for (int tx = cx - RING; tx <= cx + RING; tx++) {
      double d = std::hypot(tx - cx, ty - cy);
      if (d >= RING - 0.5 && d < RING + 0.5 && !c.edits.at(tx, ty))
        c.edits.place(tx, ty, Piece::Rubble);
    }
}

void Delve::unseal(Edits &e) {
  for (auto it = e.changes.begin(); it != e.changes.end();)
    if (it->second.placed && it->second.piece == Piece::Rubble)
      it = e.changes.erase(it);
    else
      ++it;
}

int Delve::inside(Level &level) const {
  int n = 0;
  level.enemiesNear(x, y, RING * 16, [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y;
    n += !e.removed && !e.buried && xd * xd + yd * yd < RING * 16.0 * RING * 16;
  });
  return n;
}

bool Delve::start(Level &level, Campaign &c, const Structure &site) {
  if (active)
    return false;
  active = true, floor = 0, wait = 1, rx = site.rx, ry = site.ry;
  x = site.centerX(), y = site.centerY();
  outside = level.rules.spawnRate, level.rules.spawnRate = 0;
  seal(c);
  level.enemiesNear(x, y, (RING + 1) * 16, [&](Enemy &e) {
    if (e.bossLike() || std::hypot(e.x - x, e.y - y) > (RING + 1) * 16)
      return;
    Spark::burst(level, e.x, e.bodyY(), DUST, 8, 60, true);
    e.remove();
  });
  level.add<Shockwave>(x, y, 8.0f, float(RING * 16), 0.8f, DUST);
  Spark::burst(level, x, y - 6, DUST, 24, 120, true);
  level.game.onExplosion();
  Sound::erupt.play(0.7f, 0);
  level.game.announce("THE MINE CAVES IN BEHIND YOU");
  return true;
}

void Delve::populate(Level &level, Campaign &c) {
  constexpr const char *DEPTHS[FLOORS] = {"FLOOR 1  THE TUNNELS STIR",
                                          "FLOOR 2  THE GROUND GIVES WAY",
                                          "THE DEEPEST FLOOR  SOMETHING WAKES"};
  level.game.announce(DEPTHS[floor]);
  for (const Wave &w : WAVES[floor]) {
    K kind = w.kind == K::Count && w.count ? guardian(c.act) : w.kind;
    for (int i = 0; kind != K::Count && i < w.count; i++) {
      double a = rng.range(0, TAU), r = rng.range(40, (RING - 2) * 16);
      bool boss = w.kind == K::Count;
      level.spawner.spawn(kind, x + std::cos(a) * r, y + std::sin(a) * r, true, boss);
    }
  }
}

void Delve::sift(Level &level) {
  if ((dust -= DT) > 0 || !Drift::room())
    return;
  dust = 0.08f;
  const Player &p = *level.player;
  double a = rng.range(0, TAU), r = rng.range(0, (RING - 1) * 16);
  double dx = x + std::cos(a) * r, dy = y + std::sin(a) * r;
  if (std::hypot(dx - p.x, dy - p.y) > 220)
    return;
  Drift *d = level.add<Drift>(dx, dy - 40, rng.range(-3, 3), rng.range(14, 26),
                              atlas::pt::Puff, Color{0.55f, 0.5f, 0.45f, 0.35f},
                              rng.range(1.2f, 2));
  d->grow = 0.3f, d->sway = 3, d->drag = 0.2f;
}

void Delve::quake(Level &level, int strength) {
  level.game.onExplosion();
  for (int i = 0; i < strength; i++)
    level.game.onExplosion();
  for (int i = 0; i < 6 * strength && Drift::room(); i++) {
    double a = rng.range(0, TAU), r = RING * 16 - rng.range(0, 12);
    Drift *d = level.add<Drift>(x + std::cos(a) * r, y + std::sin(a) * r - 10,
                                -std::cos(a) * 20, -std::sin(a) * 20, atlas::pt::Puff,
                                Color{0.5f, 0.46f, 0.4f, 0.55f}, rng.range(1, 1.8f));
    d->grow = 0.8f, d->drag = 1.2f;
  }
  Sound::erupt.play(0.25f + 0.15f * strength, 0.1f);
}

void Delve::tick(Level &level, Campaign &c) {
  gloom = std::clamp(gloom + (active ? DT : -DT) / 1.5f, 0.0f, 1.0f);
  if (!active)
    return;
  sift(level);
  if ((wait -= DT) > 0)
    return;
  if (inside(level) > 0)
    return;
  if (floor >= FLOORS)
    return finish(level, c);
  quake(level, floor + 1);
  populate(level, c);
  floor++, wait = PAUSE;
}

void Delve::finish(Level &level, Campaign &c) {
  active = false;
  level.rules.spawnRate = outside;
  unseal(c.edits);
  c.delves.push_back({rx, ry});
  level.add<Chest>(x, y);
  int mlvl = std::clamp(level.monsterLevel(x, y) + 3, 1, 99);
  ItemDrop::spill(level, x, y, Source::Boss, mlvl);
  for (int i = 0; i < 6; i++) {
    float a = rng.range(0, TAU), r = rng.range(10, 30);
    Mat m = i < 4 ? Mat::Ore : Mat::Rune;
    level.add<ItemDrop>(x, y, Drop{DropKind::Material, 1, {}, int(m)}, x + std::cos(a) * r,
                        y + std::sin(a) * r * 0.7f);
  }
  level.add<Shockwave>(x, y, 8, RING * 16, 0.8f, col::gold);
  level.game.announce("THE MINE IS CLEARED");
  Sound::chest.play(0.7f, 0, 0.8f);
  level.game.saveCampaign();
}

void Delve::abort(Level &level, Campaign &c) {
  if (!active)
    return;
  active = false;
  level.rules.spawnRate = outside;
  unseal(c.edits);
  level.add<Shockwave>(x, y, 6, RING * 16, 0.6f, DUST);
}
