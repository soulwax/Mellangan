#include "entity/enemy/Necromancer.hpp"

void Necromancer::renderLight(Screen &s) {
  Enemy::renderLight(s);
  s.glow(xp, yp - 7 * scale, 42 * scale, {0.14f, 0.04f, 0.22f, 1});
}
