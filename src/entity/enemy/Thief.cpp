#include "entity/enemy/Thief.hpp"

#include "Atlas.hpp"
#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/enemy/ai/Goal.hpp"
#include "entity/pickup/Chest.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

void Thief::render(Screen &s) {
  if (const Goal *g = goals.front()) {
    float t = g->tell(*this);
    if (t > 0) {
      // swirl, escape portal
      s.color(Color{0.4f, 0.9f, 1.0f, t * 0.75f});
      s.center(atlas::Ring, xp, yp - 4, 24.0f / atlas::Ring.w, animT * 5.0f);
      s.color(Color{1.0f, 0.85f, 0.3f, t * 0.65f});
      s.center(atlas::Ring, xp, yp - 4, 18.0f / atlas::Ring.w, -animT * 4.0f);
      s.color(col::white);
    }
  }
  Enemy::render(s);
}

void Thief::renderLight(Screen &s) {
  Enemy::renderLight(s);
  s.glow(xp, yp - 4, 35, Color{0.3f, 0.25f, 0.08f, 1});
}

void Thief::die() {
  Enemy::die();
  level->add<Chest>(x, y);
  Spark::burst(*level, x, bodyY(), col::gold, 35, 120, true);
  level->game.announce("THIEF CAUGHT!");
}
