#pragma once

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

namespace ability {

template <class F> inline void within(Mob &caster, float radius, F &&fn) {
  caster.level->foesOf(caster, caster.x, caster.y, radius + 20, [&](Mob &foe) {
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (xd * xd + yd * yd < radius * radius)
      fn(foe, xd, yd);
  });
}

inline void lob(Mob &caster, double x, double y, float damage, float pool,
         float burn, float time) {
  float power = caster.power();
  caster.level->add<Bomb>(caster.x, caster.bodyY(), x, y, damage * power, pool,
                          burn * power, time)->hostile = caster.hostile();
}

inline void burst(Mob &caster, float radius, float damage, Color tone) {
  caster.level->foesOf(caster, caster.x, caster.y, radius + 20, [&](Mob &foe) {
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (xd * xd + yd * yd < radius * radius)
      foe.strike(damage * caster.power(), xd, yd, 140);
  });
  Spark::burst(*caster.level, caster.x, caster.bodyY(), tone, 18, 140, true);
}

} // namespace ability
