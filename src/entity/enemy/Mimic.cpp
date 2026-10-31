#include "entity/enemy/Mimic.hpp"

#include "entity/Player.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"

void Mimic::render(Screen &s) {
  if (!buried)
    return Enemy::render(s);
  const Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  float near = std::clamp(1 - float(std::sqrt(xd * xd + yd * yd)) / 120, 0.0f, 1.0f);
  float jig = near * near * std::sin(animT * 38) * 1.5f;
  s.center(atlas::tile(atlas::t::Chest), xp + jig, yp - 4);
}

void Mimic::renderShadow(Screen &s) {
  if (!buried)
    Enemy::renderShadow(s);
}

void Mimic::renderLight(Screen &s) {
  if (!buried)
    return Enemy::renderLight(s);
  s.glow(xp, yp, 100, Color{0.42f, 0.3f, 0.1f, 1} * (1 + 0.2f * std::sin(animT * 5)));
}

void Mimic::die() {
  Enemy::die();
  if (rng.chance(0.5f))
    level->add<Potion>(x + 6, y);
}
