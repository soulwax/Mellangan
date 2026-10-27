#include "entity/enemy/Ghost.hpp"

#include "entity/enemy/Enemy.hpp"

Color Ghost::tint() const {
  const Goal *g = goals.front();
  return col::white.alpha(g && g->conceals() ? 0.28f : 0.72f);
}

void Ghost::renderLight(Screen &s) {
  Enemy::renderLight(s);
  s.glow(xp, yp - 7 * scale, 40, {0.06f, 0.10f, 0.18f, 1});
}
