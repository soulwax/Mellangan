#include "entity/enemy/Burrower.hpp"

#include <cmath>

#include "Atlas.hpp"
#include "entity/enemy/ai/Goal.hpp"
#include "gfx/Screen.hpp"
#include "gfx/EnemyWarning.hpp"

void Burrower::render(Screen &s) {
  if (buried) {
    s.center(atlas::frontier(atlas::fr::Mound), xp, yp - 3);
    if (const Goal *g = goals.front()) {
      float t = g->tell(*this);
      if (t > 0) {
        enemyWarning(s, xMark, yMark, 16, t * 0.9f, animT * 3.5f);
      }
    }
    return;
  }
  Enemy::render(s);
}

void Burrower::renderShadow(Screen &s) {
  if (!buried)
    Enemy::renderShadow(s);
}

void Burrower::renderLight(Screen &s) {
  Enemy::renderLight(s);
  if (buried) {
    if (const Goal *g = goals.front()) {
      float t = g->tell(*this);
      if (t > 0)
        s.glow(xMark, yMark, 50, Color{0.35f, 0.15f, 0.05f, 1} * t);
    }
  }
}
