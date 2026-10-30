#include "entity/pickup/Nova.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Explosion.hpp"
#include "particle/Spark.hpp"

void Nova::render(Screen &s) {
  s.center(atlas::tile(atlas::t::VialBlue), xp, yp, 0.9f);
}

void Nova::renderLight(Screen &s) {
  s.glow(xp, yp, 60,
         Color{0.3f, 0.3f, 0.45f, 1} * (0.8f + 0.2f * std::sin(t * 6)));
}

void Nova::take(Player &p) {
  level->each<Enemy>([&](Enemy &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < 260 * 260)
      e.hurt(e.boss || e.is(EnemyKind::Cyclops) ? 250.0f : 9999.0f, xd, yd, 200);
  });
  Sound::nova.play(0.8f, 0);
  Spark::burst(*level, p.x, p.bodyY(), {0.8f, 0.85f, 1, 1}, 40, 220, true);
  level->add<Explosion>(p.x, p.y, 4, 8);
  level->game.onExplosion();
}
