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

void miasma(Mob &caster, float radius, float poison, float seconds) {
  Level &level = *caster.level;
  constexpr Color TOXIC{0.55f, 0.95f, 0.35f, 1};
  within(caster, radius, [&](Mob &foe, double, double) {
    foe.afflict(StatusKind::Poison, seconds, poison * caster.power());
  });
  for (int i = 0; i < 14 && Drift::room(); i++) {
    float a = rng.range(0, TAU), r = rng.range(10, radius);
    Drift *d = level.add<Drift>(caster.x + std::cos(a) * r,
                                caster.y + std::sin(a) * r * 0.7f, rng.range(-6, 6),
                                -rng.range(4, 12), atlas::pt::Puff, TOXIC.alpha(0.4f),
                                rng.range(1.2f, 2.0f));
    d->grow = 1.6f, d->scale = 0.8f, d->spin = rng.range(-1, 1);
  }
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, radius, 0.5f, TOXIC.alpha(0.7f), false);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Bubble, TOXIC, 10, 50, -40);
  Sound::nova.play(0.4f, 0, 0.8f);
}

void grasp(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  constexpr Color DUSK{0.62f, 0.22f, 0.45f, 1};
  int n = 0;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), -xd, -yd, 480);
    foe.afflict(StatusKind::Chill, 2, 0.45f);
    if (n++ < 8)
      flourish::streak(level, foe.x, foe.bodyY(), caster.x, caster.bodyY(),
                       atlas::pt::Wisp, DUSK, 4);
  });
  flourish::sigil(level, caster.x, caster.bodyY(), radius, DUSK, 10);
  level.add<Shockwave>(caster.x, caster.bodyY(), radius, 10, 0.45f, DUSK.alpha(0.8f), false);
  Sound::scythe.play(0.6f, 0, 0.7f);
}

void entangle(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  constexpr Color THORN{0.45f, 0.8f, 0.35f, 1};
  int n = 0;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), xd, yd, 30);
    foe.afflict(StatusKind::Chill, 2.5f, 0.65f);
    if (n++ < 12)
      flourish::scatter(level, foe.x, foe.y, atlas::pt::Leaf, THORN, 3, 30, -20);
  });
  flourish::sigil(level, caster.x, caster.bodyY(), radius, THORN, 12);
  level.add<Shockwave>(caster.x, caster.y, 4, radius, 0.5f, THORN.alpha(0.7f), false);
  Sound::erupt.play(0.5f, 0);
}

void snare(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  double ax, ay;
  caster.aim(ax, ay);
  int n = 0;
  within(caster, reach, [&](Mob &foe, double xd, double yd) {
    double d = std::max(1.0, std::sqrt(xd * xd + yd * yd));
    if ((xd * ax + yd * ay) / d < 0.5)
      return;
    foe.strike(damage * caster.power(), xd, yd, 20);
    foe.afflict(StatusKind::Chill, 3, 0.65f);
    if (n++ < 10)
      flourish::scatter(level, foe.x, foe.bodyY(), atlas::pt::Fluff,
                        Color{0.85f, 0.78f, 0.6f, 1}, 2, 20);
  });
  for (int i = -2; i <= 2; i++) {
    double a = std::atan2(ay, ax) + i * 0.25;
    flourish::streak(level, caster.x, caster.bodyY(), caster.x + std::cos(a) * reach,
                     caster.bodyY() + std::sin(a) * reach, atlas::pt::Fluff,
                     Color{0.85f, 0.78f, 0.6f, 0.8f}, 4);
  }
  Sound::bow.play(0.6f, 0, 0.8f);
}

void lodestar(Mob &caster, float radius) {
  Level &level = *caster.level;
  if (caster.asPlayer())
    level.each<Gem>([&](Gem &g) {
      double xd = g.x - caster.x, yd = g.y - caster.y;
      if (!g.magnet && xd * xd + yd * yd < radius * radius)
        g.magnet = true, g.speed = -60, g.swirl = rng.range(-1, 1);
    });
  level.add<Shockwave>(caster.x, caster.bodyY(), radius * 0.4f, 6, 0.6f,
                       Color{0.5f, 0.85f, 1, 0.7f}, false);
  flourish::sigil(level, caster.x, caster.bodyY(), 36, Color{0.5f, 0.85f, 1, 1}, 6);
  Sound::magnet.play(0.6f, 0);
}

void eclipse(Mob &caster, float radius, float seconds) {
  constexpr Color DUSK{0.6f, 0.5f, 0.95f, 1};
  caster.level->add<Aura>(caster, AuraDef{radius, seconds, 0, 0, 0.3f, 0, 0, 0.35f,
                                          0, 0, true, DUSK, atlas::pt::ArcaneStar});
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 46, DUSK, 10);
  Sound::cast.play(0.5f, 0, 0.7f);
}

void overgrowth(Mob &caster, float radius, float seconds) {
  constexpr Color WILD{0.45f, 0.85f, 0.35f, 1};
  caster.level->add<Aura>(caster, AuraDef{radius, seconds, 0, 0, 0.45f,
                                          4 * caster.power(), 0, 0, 0, 0, false,
                                          WILD, atlas::pt::Leaf});
  flourish::scatter(*caster.level, caster.x, caster.bodyY(), atlas::pt::Leaf, WILD,
                    12, 80, -20);
  Sound::erupt.play(0.45f, 0, 1.1f);
}

} // namespace ability
