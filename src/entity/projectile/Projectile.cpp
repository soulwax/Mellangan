#include "entity/projectile/Projectile.hpp"

#include <algorithm>

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

void Projectile::collide() {
  if (hostile || (source && source->hostile())) {
    Player &p = *level->player;
    if (removed || p.dead() || !touches(p) ||
        std::find(hits.begin(), hits.end(), p.id) != hits.end())
      return;
    hits.push_back(p.id);
    double xd, yd;
    push(p, xd, yd);
    p.strike(damage, xd, yd, knock);
    if (--pierce <= 0)
      spent(), remove();
    return;
  }
  level->enemiesNear(x, y, radius + 32, [&](Enemy &e) {
    if (removed || !touches(e) ||
        std::find(hits.begin(), hits.end(), e.id) != hits.end())
      return;
    hits.push_back(e.id);
    double xd, yd;
    push(e, xd, yd);
    e.hurt(damage, xd, yd, knock, source);
    if (--pierce <= 0)
      spent(), remove();
  });
}

bool Projectile::touches(const Mob &e) const {
  double xd = e.x - x, yd = e.bodyY() - y;
  float r = radius + e.radius;
  return xd * xd + yd * yd < r * r;
}
