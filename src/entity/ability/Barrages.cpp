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

void mines(Mob &caster, int count, float damage) {
  for (int i = 0; i < count; i++) {
    double dir = TAU * i / count + rng.range(-0.2f, 0.2f), r = rng.range(60, 95);
    lob(caster, caster.x + std::cos(dir) * r, caster.y + std::sin(dir) * r, damage,
        22, damage * 0.25f, 2.5f);
  }
  caster.level->add<Shockwave>(caster.x, caster.bodyY(), 4, 30, 0.3f,
                               Color{0.95f, 0.7f, 0.35f, 0.8f}, false);
  Sound::axe.play(0.6f, 0, 0.8f);
}

void comet(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  bool h = caster.hostile();
  auto targets = level.nearestFoes(caster, caster.x, caster.y, 260, count);
  for (int i = 0; i < count; i++) {
    double x = caster.x + rng.range(-110, 110), y = caster.y + rng.range(-80, 80);
    if (!targets.empty()) {
      Mob &t = *targets[i % targets.size()];
      x = t.x + (h ? rng.range(-40, 40) : 0), y = t.y + (h ? rng.range(-28, 28) : 0);
    }
    float power = caster.power();
    level.add<MeteorRock>(x, y, damage * power, 30, damage * 0.2f * power)->hostile = h;
  }
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::FallingStar,
                    Color{0.85f, 0.75f, 1, 1}, 6, 60, -40);
  Sound::cast.play(0.6f, 0, 0.8f);
}

void keg(Mob &caster, float damage) {
  double ax, ay;
  caster.aim(ax, ay);
  double x = caster.x + ax * 110, y = caster.y + ay * 110;
  auto near = caster.level->nearestFoes(caster, caster.x, caster.y, 170, 1);
  if (caster.hostile() && !near.empty())
    x = near[0]->x, y = near[0]->y;
  lob(caster, x, y, damage, 46, damage * 0.15f, 3.5f);
  Sound::axe.play(0.7f, 0, 0.7f);
}

void firering(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  for (int i = 0; i < count; i++) {
    double a = TAU * i / count;
    level.add<FirePool>(caster.x + std::cos(a) * 55, caster.y + std::sin(a) * 40, 20,
                        damage * caster.power(), 4)->hostile = caster.hostile();
  }
  Spark::burst(level, caster.x, caster.bodyY(), Color{1, 0.5f, 0.15f, 1}, 14, 110, true);
  Sound::explosion.play(0.35f, 0, 1.3f);
}

void hunt(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  constexpr Color HUNT{0.85f, 1, 0.55f, 1};
  for (Mob *e : level.nearestFoes(caster, caster.x, caster.y, 260, count)) {
    e->afflict(StatusKind::Marked, 6, 0.4f);
    double xd = e->x - caster.x, yd = e->bodyY() - caster.bodyY(),
           d = std::max(1.0, std::sqrt(xd * xd + yd * yd));
    level.add<Arrow>(caster.x, caster.bodyY(), xd / d * 280, yd / d * 280,
                     damage * caster.power(), 1)->hostile = caster.hostile();
    flourish::sigil(level, e->x, e->bodyY(), 10, HUNT, 4);
  }
  Sound::bow.play(0.7f, 0, 1.1f);
}

void barrage(Mob &caster, int count, float damage) {
  auto targets = caster.level->nearestFoes(caster, caster.x, caster.y, 220, count);
  for (int i = 0; i < count; i++) {
    double x = caster.x + rng.range(-90, 90), y = caster.y + rng.range(-70, 70);
    if (!targets.empty()) {
      Mob &t = *targets[size_t(i) % targets.size()];
      x = t.x + rng.range(-8, 8), y = t.y + rng.range(-6, 6);
    }
    lob(caster, x, y, damage, 18, damage * 0.2f, 2);
  }
  flourish::scatter(*caster.level, caster.x, caster.bodyY(), atlas::pt::Bubble,
                    Color{1, 0.6f, 0.3f, 1}, 6, 50, -30);
  Sound::potion.play(0.6f, 0, 0.8f);
}

void tempest(Mob &caster, float seconds, float damage) {
  constexpr Color STORM{0.55f, 0.75f, 1, 1};
  caster.level->add<Aura>(caster, AuraDef{120, seconds, 0, 0, 0, 0, 0, 0, 0.35f,
                                          damage * caster.power(), true, STORM,
                                          atlas::pt::Gust});
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 50, STORM, 10);
  Sound::thunder.play(0.5f);
}

} // namespace ability
