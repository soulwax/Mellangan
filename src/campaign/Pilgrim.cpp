#include "campaign/Pilgrim.hpp"

#include <cmath>

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr int PILGRIM = 111;
constexpr Color LANTERN{1, 0.8f, 0.45f, 1};

} // namespace

void Pilgrim::onAdded() { level->tag(this); }

bool Pilgrim::arrived() const { return std::hypot(tx - x, ty - y) < ARRIVE; }

void Pilgrim::tick() {
  t += DT;
  flash = std::max(0.0f, flash - DT);
  Player &p = *level->player;
  level->enemiesNear(x, y, 24, [&](Enemy &e) {
    if (e.buried || e.removed || std::hypot(e.x - x, e.y - y) > e.type.radius + 7)
      return;
    float bite = e.type.damage * e.damageMul * DT * 1.5f;
    if (bite > 0)
      hp -= bite, flash = 0.15f;
  });
  if (hp <= 0) {
    Spark::burst(*level, x, y - 8, LANTERN, 16, 70, true);
    return remove();
  }
  waiting = p.dead() || std::hypot(p.x - x, p.y - y) > WAIT;
  if (waiting || arrived())
    return;
  double xd = tx - x, yd = ty - y, d = std::hypot(xd, yd);
  x += xd / d * SPEED * DT, y += yd / d * SPEED * DT;
  facing = xd < 0 ? -1.0f : 1.0f;
  level->collide(x, y, 5);
}

void Pilgrim::render(Screen &s) {
  float bob = waiting ? 0 : std::abs(std::sin(t * 7)) * 1.2f;
  s.color(flash > 0 ? Color{1, 0.5f, 0.5f, 1} : col::white);
  s.sprite(atlas::tile(PILGRIM), xp, yp - bob, 8, 16, facing, 1);
  s.color(col::white);
  float k = std::clamp(hp / HP, 0.0f, 1.0f);
  s.rect(xp - 8, yp - 21, 16, 2, Color{0.1f, 0.05f, 0.08f, 0.8f});
  s.rect(xp - 8, yp - 21, 16 * k, 2, Color{0.5f, 0.9f, 0.5f, 0.9f});
  if (waiting && std::fmod(t, 1.2f) < 0.8f)
    s.rect(xp - 1, yp - 27, 2, 4, LANTERN);
}

void Pilgrim::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 0.9f, 0.7f);
}

void Pilgrim::renderLight(Screen &s) {
  s.glow(xp + facing * 5, yp - 8, 60, LANTERN * 0.3f);
  level->lights.add(x, y, 56, 0.5f);
}
