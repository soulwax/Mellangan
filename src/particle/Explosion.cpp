#include "particle/Explosion.hpp"

#include "level/Level.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"

Explosion::Explosion(double x, double y, double z, int life)
    : life(life), maxLife(life) {
  this->x = x, this->y = y, this->z = z;
}

void Explosion::tick() {
  if (--life < 0)
    return remove();
  int ps = life * 40 / maxLife + 1;
  double dd = double(maxLife - life) / maxLife + 0.2;
  for (int i = 0; i < ps && Debris::room(); i++) {
    double dir = rng.range(0, TAU), dist = rng.range(0, 6) * dd;
    double xx = x + std::cos(dir) * dist, yy = y + std::sin(dir) * dist,
           zz = z + rng.range(0, 10);
    auto fd = std::make_unique<FlameDebris>(xx, yy, zz);
    if (rng.chance(0.5f))
      fd->life = fd->maxLife / 2;
    fd->xa = fd->xa * 0.1 + (xx - x) * 0.5;
    fd->ya = fd->ya * 0.1 + (yy - y) * 0.5;
    fd->za *= 0.1;
    fd->gravity = 0.10;
    level->spawn(std::move(fd));
  }
}
