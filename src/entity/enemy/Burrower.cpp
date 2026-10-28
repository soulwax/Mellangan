#include "entity/enemy/Burrower.hpp"

#include <cmath>

#include "Atlas.hpp"
#include "entity/enemy/ai/Goal.hpp"
#include "gfx/Screen.hpp"

void Burrower::render(Screen &s) {
  if (buried) {
    s.center(atlas::frontier(atlas::fr::Mound), xp, yp - 3);
    if (const Goal *g = goals.front()) {
      float t = g->tell(*this);
      if (t > 0) {
        s.color(Color{0.95f, 0.45f, 0.15f, t * 0.85f});
        s.center(atlas::Ring, xMark, yMark, 32.0f / atlas::Ring.w, animT * 3.5f);
        s.color(col::white);
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
