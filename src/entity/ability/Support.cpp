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

void rally(Mob &caster, float reach, float damage, float guard) {
  Level &level = *caster.level;
  level.add<Sweep>(caster, reach, damage * caster.power(), SweepStyle::Rally);
  caster.shield(guard);
  level.add<Ward>(caster, guard);
  flourish::sigil(level, caster.x, caster.bodyY(), 70, col::gold, 12);
  Spark::burst(level, caster.x, caster.bodyY(), col::gold, 10, 120, true);
  Sound::scythe.play(0.6f, 0);
}

void warcry(Mob &caster, float radius, float seconds) {
  Level &level = *caster.level;
  constexpr Color RAGE{1, 0.4f, 0.3f, 1};
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(6 * caster.power(), xd, yd, 260);
    foe.afflict(StatusKind::Marked, seconds, 0.3f);
  });
  level.add<Shockwave>(caster.x, caster.bodyY(), 8, radius, 0.4f, RAGE.alpha(0.85f));
  level.add<Shockwave>(caster.x, caster.bodyY(), 4, radius * 0.7f, 0.55f,
                       RAGE.alpha(0.5f), false);
  flourish::sigil(level, caster.x, caster.bodyY(), 40, RAGE, 10);
  Sound::howl.play(0.6f, 0, 1.15f);
}

void bandage(Mob &caster, float amount) {
  caster.mend(amount);
  for (StatusKind k : {StatusKind::Burn, StatusKind::Poison, StatusKind::Chill})
    caster.status.end(k);
  flourish::scatter(*caster.level, caster.x, caster.bodyY(), atlas::pt::HolyGlint,
                    Color{0.5f, 1, 0.6f, 1}, 10, 40, -50);
  Spark::burst(*caster.level, caster.x, caster.bodyY(), Color{0.5f, 1, 0.6f, 1}, 8,
               60, true);
  Sound::potion.play(0.6f, 0);
}

void tumble(Mob &caster, float reach) {
  Level &level = *caster.level;
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  flourish::echo(level, caster.x, caster.y, caster.body(), Color{0.8f, 0.85f, 1, 1});
  caster.x += ax * reach, caster.y += ay * reach;
  level.collide(caster.x, caster.y, caster.radius);
  caster.shield(0.6f);
  flourish::streak(level, x0, y0, caster.x, caster.bodyY(), atlas::pt::Puff,
                   Color{0.85f, 0.8f, 0.7f, 0.6f}, 5);
  Sound::slash.play(0.4f, 0, 1.4f);
}

void banner(Mob &caster, float radius, float seconds) {
  constexpr Color GOLD{1, 0.85f, 0.45f, 1};
  caster.level->add<Aura>(caster, AuraDef{radius, seconds, 4, 0, 0.35f, 0, 0, 0, 0,
                                          0, false, GOLD, atlas::pt::HolyGlint});
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 40, GOLD, 8);
  caster.level->add<Shockwave>(caster.x, caster.y, 4, radius, 0.4f, GOLD.alpha(0.8f),
                               false);
  Sound::raise.play(0.5f, 0, 1.2f);
}

void sanctuary(Mob &caster, float radius, float seconds) {
  constexpr Color HOLY{1, 0.95f, 0.75f, 1};
  caster.level->add<Aura>(caster, AuraDef{radius, seconds, 6, 0, 0, 0,
                                          4 * caster.power(), 0, 0, 0, false, HOLY,
                                          atlas::pt::HolyGlint});
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), radius * 0.6f, HOLY, 12);
  Sound::nova.play(0.5f, 0, 1.3f);
}

} // namespace ability
