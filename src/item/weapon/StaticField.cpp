#include "item/weapon/StaticField.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

StaticField::StaticField()
    : Upgrade("Static Field", atlas::tile(atlas::t::StormStaff),
              {"A crackling field shocks foes close by", "+3 damage",
               "Wider field", "Faster pulses", "+3 damage, wider field"},
              true) {
  owner = "STORMCALLER";
}

void StaticField::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 0.8f : 1.1f) * p.haste * pace;
  float r = 50.0f + 12 * (at(level, 3) + at(level, 5)),
        dmg = (6.0f + 3 * (at(level, 2) + at(level, 5))) * p.might * power;
  p.level->enemiesNear(p.x, p.y, r + 24, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd < rr * rr)
      e.hurt(dmg, xd, yd, 15, this);
  });
  Spark::burst(*p.level, p.x, p.bodyY(), {0.5f, 0.78f, 1, 1}, 8, r * 1.6f,
               true);
}
