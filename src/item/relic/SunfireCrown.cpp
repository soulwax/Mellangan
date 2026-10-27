#include "item/relic/SunfireCrown.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Explosion.hpp"
#include "particle/Spark.hpp"

SunfireCrown::SunfireCrown()
    : Upgrade("Sunfire Crown", atlas::kit(atlas::kt::Sunflower),
              {"A solar flare scorches all around you", "", "", "", ""}, true) {
  rare = true;
}

void SunfireCrown::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = 3 * p.haste;
  float r = 140, dmg = 80 * p.might;
  p.level->enemiesNear(p.x, p.y, r + 24, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd < rr * rr)
      e.hurt(dmg, xd, yd, 160, this);
  });
  Spark::burst(*p.level, p.x, p.bodyY(), col::gold, 36, 200, true);
  p.level->add<Explosion>(p.x, p.y, 4, 8);
  p.level->game.onExplosion();
  Sound::nova.play(0.45f);
}
