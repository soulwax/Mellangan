#include "item/weapon/FrostNova.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

FrostNova::FrostNova()
    : Upgrade("Frost Nova", atlas::kit(atlas::kt::FrozenShrub),
              {"Frost blasts slow foes by 35% for 2s", "+8 damage", "Wider burst",
               "Faster bursts", "+8 damage, wider burst"},
              true) {
  owner = "MAGE";
}

void FrostNova::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 2.6f : 3.4f) * p.haste * pace;
  float r = 60.0f + 14 * (at(level, 3) + at(level, 5)),
        dmg = (18.0f + 8 * (at(level, 2) + at(level, 5))) * p.might * power;
  p.level->enemiesNear(p.x, p.y, r + 24, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd < rr * rr) {
      e.afflict(StatusKind::Chill, 2.0f, 0.35f, this);
      e.hurt(dmg, xd, yd, 180, this);
    }
  });
  Spark::burst(*p.level, p.x, p.bodyY(), {0.6f, 0.85f, 1, 1}, 24, r * 2.2f,
               true);
  Sound::cast.play(0.35f);
}
