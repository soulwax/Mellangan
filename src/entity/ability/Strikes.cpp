#include "entity/Ability.hpp"
#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "level/Level.hpp"
#include "Game.hpp"
#include "entity/effect/Aura.hpp"
#include "entity/effect/FirePool.hpp"
#include "entity/effect/GlaiveThrust.hpp"
#include "entity/effect/Smite.hpp"
#include "entity/effect/Strike.hpp"
#include "entity/effect/Sweep.hpp"
#include "entity/projectile/Slash.hpp"
#include "entity/effect/Ward.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Bomb.hpp"
#include "entity/projectile/MeteorRock.hpp"
#include "entity/pickup/Gem.hpp"
#include "particle/Arc.hpp"
#include "particle/debris/Chip.hpp"
#include "particle/Mote.hpp"
#include "particle/Drift.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "entity/ability/Common.hpp"

namespace ability {

void blink(Mob &caster, float reach, float damage, Color tone) {
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  burst(caster, 60, damage, tone);
  flourish::echo(*caster.level, caster.x, caster.y, caster.body(), tone);
  caster.x += ax * reach, caster.y += ay * reach;
  caster.level->collide(caster.x, caster.y, caster.radius);
  flourish::streak(*caster.level, x0, y0, caster.x, caster.bodyY(),
                   atlas::pt::ArcaneStar, tone, 7);
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 60, tone, 6);
  burst(caster, 60, damage, tone);
  Sound::cast.play(0.6f, 0);
}

void nova(Mob &caster, float radius, float damage, float heal, Color tone) {
  caster.mend(heal);
  caster.level->foesOf(caster, caster.x, caster.y, radius + 20, [&](Mob &foe) {
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (xd * xd + yd * yd < radius * radius)
      foe.strike(damage * caster.power(), xd, yd, 220);
  });
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), radius, tone, 16);
  flourish::scatter(*caster.level, caster.x, caster.bodyY(), atlas::pt::HolyGlint,
                    tone, 12, 40, -50);
  Spark::burst(*caster.level, caster.x, caster.bodyY(), tone, 12, 170, true);
  Sound::nova.play(0.5f, 0);
}

void reap(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  level.add<Sweep>(caster, reach, damage * caster.power());
  int caught = 0;
  level.foesOf(caster, caster.x, caster.y, reach + 8, [&](Mob &foe) {
    if (caught++ < 10)
      level.add<Mote>(foe.x, foe.bodyY(), atlas::pt::Wisp,
                      Color{1, 0.55f, 0.62f, 0.95f}, rng.range(0.5f, 0.8f));
  });
  caster.mend(2.0f * caught);
  flourish::sigil(level, caster.x, caster.bodyY(), reach, Color{0.9f, 0.25f, 0.32f, 1}, 0);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Puff,
                    Color{0.22f, 0.08f, 0.14f, 0.6f}, 8, 70);
  Spark::burst(level, caster.x, caster.bodyY(), {0.9f, 0.3f, 0.35f, 1}, 10, 150, true);
  Sound::scythe.play(0.7f, 0);
}

void dance(Mob &caster, float reach, float damage, Color tone) {
  Level &level = *caster.level;
  auto cut = [&] {
    for (float side : {1.0f, -1.0f})
      level.add<Slash>(caster, side, damage * caster.power(), 1.2f,
                       SlashStyle::Flurry);
  };
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  cut();
  flourish::echo(level, caster.x, caster.y, caster.body(), tone);
  caster.x += ax * reach, caster.y += ay * reach;
  level.collide(caster.x, caster.y, caster.radius);
  caster.shield(0.5f);
  flourish::streak(level, x0, y0, caster.x, caster.bodyY(), atlas::pt::Petal,
                   col::white, 9);
  cut();
  Spark::burst(level, caster.x, caster.bodyY(), tone, 14, 120, true);
  Sound::slash.play(0.7f, 0);
}

void charge(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  double x0 = caster.x, y0 = caster.y, ax, ay;
  caster.aim(ax, ay);
  flourish::echo(level, caster.x, caster.y, caster.body(), col::gold);
  caster.x += ax * reach, caster.y += ay * reach;
  level.collide(caster.x, caster.y, caster.radius);
  double vx = caster.x - x0, vy = caster.y - y0,
         len = std::max(1.0, vx * vx + vy * vy);
  level.foesOf(caster, (x0 + caster.x) / 2, (y0 + caster.y) / 2, reach / 2 + 30,
               [&](Mob &foe) {
                 double b = std::clamp(((foe.x - x0) * vx + (foe.y - y0) * vy) / len, 0.0, 1.0);
                 double xd = foe.x - x0 - vx * b, yd = foe.y - y0 - vy * b;
                 float r = 14 + foe.radius;
                 if (xd * xd + yd * yd < r * r)
                   foe.strike(damage * caster.power(), ax + xd * 0.05, ay + yd * 0.05, 260);
               });
  caster.shield(0.4f);
  flourish::streak(level, x0, y0 - 6, caster.x, caster.bodyY(), atlas::pt::Pebble,
                   Color{0.85f, 0.88f, 0.95f, 1}, 8);
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, 46, 0.35f, col::gold.alpha(0.8f));
  Spark::burst(level, caster.x, caster.bodyY(), col::gold, 10, 110, true);
  Sound::scythe.play(0.6f, 0, 0.8f);
}

void lunge(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  flourish::echo(level, caster.x, caster.y, caster.body(), col::white);
  caster.x += ax * reach * 0.4, caster.y += ay * reach * 0.4;
  level.collide(caster.x, caster.y, caster.radius);
  level.add<GlaiveThrust>(caster, ax, ay, reach, damage * caster.power(), 180, false);
  caster.shield(0.3f);
  flourish::streak(level, x0, y0, caster.x, caster.bodyY(), atlas::pt::Petal,
                   col::white, 5);
  Sound::slash.play(0.7f, 0, 0.85f);
}

void whirlwind(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  level.add<Sweep>(caster, reach, damage * caster.power());
  level.add<Shockwave>(caster.x, caster.bodyY(), reach * 0.5f, reach, 0.3f,
                       Color{0.85f, 0.9f, 1, 0.7f}, false);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Gust, col::white, 6,
                    90);
  Sound::scythe.play(0.6f, 0, 1.2f);
}

void quake(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), xd, yd, 300);
    foe.afflict(StatusKind::Chill, 1.2f, 0.65f);
  });
  for (int i = 0; i < 10 && Debris::room(); i++) {
    float a = rng.range(0, TAU), r = rng.range(10, radius);
    level.add<Chip>(caster.x + std::cos(a) * r, caster.y + std::sin(a) * r * 0.7f, 2,
                    i % 3 ? atlas::pt::Pebble : atlas::pt::Rock, col::white, 1.2);
  }
  level.add<Shockwave>(caster.x, caster.y, 6, radius, 0.45f,
                       Color{0.75f, 0.6f, 0.4f, 0.9f});
  level.game.onExplosion();
  Sound::explosion.play(0.5f, 0, 0.7f);
}

void drain(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  constexpr Color BLOOD{0.85f, 0.2f, 0.3f, 1};
  int n = 0;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), xd, yd, 40);
    if (n++ < 8)
      flourish::streak(level, foe.x, foe.bodyY(), caster.x, caster.bodyY(),
                       atlas::pt::Wisp, BLOOD, 4);
  });
  if (n)
    caster.mend(std::min(40.0f, 3.0f * n));
  level.add<Shockwave>(caster.x, caster.bodyY(), radius, 8, 0.45f, BLOOD.alpha(0.8f),
                       false);
  Sound::scythe.play(0.6f, 0, 0.6f);
}

void chain(Mob &caster, int jumps, float reach, float damage) {
  Level &level = *caster.level;
  constexpr Color STORM{0.55f, 0.75f, 1, 1};
  auto targets = level.nearestFoes(caster, caster.x, caster.y, reach, jumps);
  if (targets.empty())
    return;
  std::vector<ArcNode> path{{caster.x, caster.bodyY()}};
  while (!targets.empty()) {
    auto next = std::min_element(targets.begin(), targets.end(), [&](Mob *a, Mob *b) {
      auto d = [&](Mob *m) {
        double xd = m->x - path.back().x, yd = m->bodyY() - path.back().y;
        return xd * xd + yd * yd;
      };
      return d(a) < d(b);
    });
    Mob &foe = **next;
    targets.erase(next);
    path.push_back({foe.x, foe.bodyY()});
    foe.strike(damage * caster.power() * caster.arcs, foe.x - caster.x, foe.y - caster.y, 60);
    Spark::burst(level, foe.x, foe.bodyY(), STORM, 5, 70, false);
  }
  level.add<Arc>(std::move(path), 0.3f, STORM);
  Sound::thunder.play(0.4f);
}

void smite(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  bool h = caster.hostile();
  auto targets = level.nearestFoes(caster, caster.x, caster.y, 240, count);
  for (int i = 0; i < count; i++) {
    double x = caster.x + rng.range(-90, 90), y = caster.y + rng.range(-70, 70);
    if (!targets.empty()) {
      Mob &t = *targets[i % targets.size()];
      x = t.x + (h ? rng.range(-34, 34) : 0), y = t.y + (h ? rng.range(-24, 24) : 0);
    }
    level.add<Smite>(x, y, damage * caster.power(), 22)->hostile = h;
  }
  flourish::sigil(level, caster.x, caster.bodyY(), 50, Color{1, 0.85f, 0.4f, 1}, 8);
  Sound::nova.play(0.45f, 0, 1.2f);
}

void flash(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  auto targets = level.nearestFoes(caster, caster.x, caster.y, 200, count);
  if (targets.empty())
    return;
  double px = caster.x, py = caster.y;
  flourish::echo(level, caster.x, caster.y, caster.body(), col::white);
  for (Mob *e : targets) {
    flourish::streak(level, px, py - 6, e->x, e->bodyY(), atlas::pt::Petal,
                     Color{1, 0.85f, 0.95f, 1}, 4);
    e->strike(damage * caster.power(), e->x - px, e->y - py, 120);
    e->afflict(StatusKind::Marked, 3, 0.3f);
    px = e->x, py = e->y;
  }
  Mob &last = *targets.back();
  double xd = caster.x - last.x, yd = caster.y - last.y,
         d = std::max(1.0, std::sqrt(xd * xd + yd * yd));
  caster.x = last.x + xd / d * 16, caster.y = last.y + yd / d * 16;
  level.collide(caster.x, caster.y, caster.radius);
  caster.shield(0.5f);
  Sound::slash.play(0.7f, 0, 1.25f);
}

} // namespace ability
