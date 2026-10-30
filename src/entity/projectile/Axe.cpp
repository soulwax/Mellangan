#include "entity/projectile/Axe.hpp"

#include "entity/projectile/Projectile.hpp"
#include "level/Level.hpp"
#include "particle/debris/Chip.hpp"

Axe::Axe(double x, double y, double xa, double ya, float damage, int pierce,
         float spin)
    : Projectile(x, y, xa, ya, 2.2f, damage, 60, pierce), spin(spin) {
  radius = 8;
}

void Axe::tick() {
  if ((life -= DT) <= 0)
    return remove();
  ya += 560 * DT;
  x += xa * DT;
  y += ya * DT;
  rot += spin * DT;
  collide();
  if (hits.size() > chopped) {
    chopped = hits.size();
    for (int i = 0; i < 2 && Debris::room(); i++)
      level->add<Chip>(x, y, 6, atlas::pt::Pebble, Color{1, 0.8f, 0.55f, 1},
                       0.9);
  }
}

void Axe::render(Screen &s) {
  for (int i = 2; i >= 1; i--) {
    s.color(Color{1, 0.9f, 0.8f, 0.2f * float(3 - i)});
    s.center(atlas::tile(atlas::t::Axe), xp - xa * 0.012 * i,
             yp - ya * 0.012 * i, 1, rot - spin * 0.025f * i);
  }
  s.color(col::white);
  s.center(atlas::tile(atlas::t::Axe), xp, yp, 1, rot);
}

void Axe::renderLight(Screen &s) {
  s.glow(xp, yp, 44, {0.20f, 0.10f, 0.03f, 1});
}
