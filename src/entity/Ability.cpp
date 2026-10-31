#include "entity/Ability.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"
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

} // namespace ability
