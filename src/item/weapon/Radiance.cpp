#include "item/weapon/Radiance.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

Radiance::Radiance()
    : Upgrade("Radiance", atlas::kit(atlas::kt::Daisies),
              {"A holy nova that mends you per foe", "+6 damage", "Wider nova",
               "Faster novas", "+6 damage, wider nova"},
              true) {
  owner = "PRIESTESS";
}

void Radiance::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 2.4f : 3.2f) * p.haste * pace;
  float r = 55.0f + 12 * (at(level, 3) + at(level, 5)),
        dmg = (14.0f + 6 * (at(level, 2) + at(level, 5))) * p.might * power;
  int hit = 0;
  p.level->enemiesNear(p.x, p.y, r + 24, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd < rr * rr)
      e.hurt(dmg, xd, yd, 90, this), hit++;
  });
  p.hp = std::min(p.maxHp, p.hp + 0.6f * std::min(hit, 10));
  Spark::burst(*p.level, p.x, p.bodyY(), {1, 0.92f, 0.6f, 1}, 20, r * 2,
               true);
  Sound::cast.play(0.3f);
}
