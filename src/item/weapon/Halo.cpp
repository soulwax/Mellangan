#include "item/weapon/Halo.hpp"

#include "entity/Player.hpp"
#include "entity/effect/HaloField.hpp"
#include "level/Level.hpp"

Halo::Halo()
    : Upgrade("Warding Halo", atlas::tile(atlas::t::PotionGrey),
              {"Scorches foes that get close", "Wider and hotter",
               "Wider and hotter", "Wider, pulses faster", "Wider and hotter"},
              true) {}

void Halo::levelUp(Player &p) {
  Upgrade::levelUp(p);
  if (level == 1)
    p.level->add<HaloField>(*this, p);
}

void Halo::tick(Player &p) {
  pulse *= std::exp(-6 * DT);
  if ((timer -= DT) > 0)
    return;
  timer = (level >= 4 ? 0.38f : 0.5f) * p.haste * pace;
  pulse = 1;
  float r = range(),
        dmg =
            (5.0f + 2 * (at(level, 2) + at(level, 3) + at(level, 5))) * p.might * power;
  p.level->enemiesNear(p.x, p.y, r + 24, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd < rr * rr)
      e.hurt(dmg, e.x - p.x, e.y - p.y, 25, this);
  });
}
