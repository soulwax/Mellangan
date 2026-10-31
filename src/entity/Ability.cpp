#include "entity/Ability.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "level/Level.hpp"
#include "Game.hpp"
#include "entity/effect/Strike.hpp"
#include "entity/effect/Sweep.hpp"
#include "entity/projectile/Slash.hpp"
#include "entity/effect/Ward.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Bomb.hpp"
#include "particle/Mote.hpp"
#include "particle/Drift.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace ability {

namespace {
void burst(Mob &caster, float radius, float damage, Color tone) {
  caster.level->foesOf(caster, caster.x, caster.y, radius + 20, [&](Mob &foe) {
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (xd * xd + yd * yd < radius * radius)
      foe.strike(damage * caster.power(), xd, yd, 140);
  });
  Spark::burst(*caster.level, caster.x, caster.bodyY(), tone, 18, 140, true);
}
} // namespace

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

void volley(Mob &caster, int count, double spread, float speed, float damage,
            int pierce) {
  double ax, ay;
  caster.aim(ax, ay);
  double aim = std::atan2(ay, ax);
  Level &level = *caster.level;
  for (int i = 0; i < count; i++) {
    double dir = aim + (i - (count - 1) / 2.0) * spread;
    Arrow *a = level.add<Arrow>(caster.x, caster.bodyY(), std::cos(dir) * speed,
                                std::sin(dir) * speed, damage * caster.power(),
                                pierce);
    a->hostile = caster.hostile();
    if (i % 2 == 0 && Drift::room()) {
      Drift *d = level.add<Drift>(caster.x, caster.bodyY(), std::cos(dir) * 110,
                                  std::sin(dir) * 110, atlas::pt::Leaf, col::white,
                                  rng.range(0.5f, 0.8f));
      d->spin = rng.range(-4, 4), d->sway = 10, d->gravity = 20;
    }
  }
  level.add<Shockwave>(caster.x, caster.bodyY(), 8, 40, 0.3f,
                       Color{0.6f, 0.95f, 0.5f, 0.7f}, false);
  Sound::bow.play(0.7f, 0);
}

void rally(Mob &caster, float reach, float damage, float guard) {
  Level &level = *caster.level;
  level.add<Sweep>(caster, reach, damage * caster.power(), SweepStyle::Rally);
  caster.shield(guard);
  level.add<Ward>(caster, guard);
  flourish::sigil(level, caster.x, caster.bodyY(), 70, col::gold, 12);
  Spark::burst(level, caster.x, caster.bodyY(), col::gold, 10, 120, true);
  Sound::scythe.play(0.6f, 0);
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

void bombs(Mob &caster, int count, float damage, float heal) {
  Level &level = *caster.level;
  caster.mend(heal);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Bubble, col::white,
                    10, 50, -60);
  level.add<Shockwave>(caster.x, caster.bodyY(), 4, 36, 0.35f,
                       Color{0.6f, 1, 0.4f, 0.8f}, false);
  double ax, ay;
  caster.aim(ax, ay);
  double aim = std::atan2(ay, ax);
  for (int i = 0; i < count; i++) {
    double dir = caster.hostile() ? aim + (i - (count - 1) / 2.0) * 0.5
                                  : TAU * i / count + rng.range(0, 1),
           r = rng.range(50, 90);
    float power = caster.power();
    Bomb *b = level.add<Bomb>(caster.x, caster.bodyY(), caster.x + std::cos(dir) * r,
                              caster.y + std::sin(dir) * r, damage * power, 28,
                              damage * 0.35f * power, 3);
    b->hostile = caster.hostile();
  }
  Sound::axe.play(0.6f, 0);
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

void thunder(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  std::vector<Mob *> near;
  level.foesOf(caster, caster.x, caster.y, 240, [&](Mob &foe) {
    if (near.size() < 48)
      near.push_back(&foe);
  });
  for (int i = 0; i < count; i++) {
    double x = caster.x + rng.range(-120, 120), y = caster.y + rng.range(-90, 90);
    if (!near.empty()) {
      Mob &t = *near[i % near.size()];
      x = t.x + (caster.hostile() ? rng.range(-30, 30) : 0);
      y = t.y + (caster.hostile() ? rng.range(-20, 20) : 0);
    }
    level.add<Strike>(x, y, damage * caster.power(), 24)->hostile = caster.hostile();
  }
  Sound::thunder.play(0.5f);
}

} // namespace ability
