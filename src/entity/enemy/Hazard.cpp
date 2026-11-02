#include "entity/enemy/Hazard.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "gfx/EnemyWarning.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

void Blast::tick() {
  if (life - DT > 0) {
    life -= DT;
    return;
  }
  remove();
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  if (hostile && xd * xd + yd * yd < radius * radius && !p.dead()) {
    p.felledBy = by;
    p.hurt(damage);
  }
  if (!hostile)
    level->enemiesNear(x, y, radius + 8, [&](Enemy &e) {
      double ex = e.x - x, ey = e.y - y;
      if (ex * ex + ey * ey < radius * radius)
        e.hurt(damage, ex, ey, 80);
    });
  level->add<Shockwave>(x, y, 4, radius, 0.3f, tone)->spokes = 8;
  Spark::burst(*level, x, y - 4, tone, 14, 120, true);
  Sound::explosion.play(0.35f);
  level->game.onExplosion();
}

void Blast::render(Screen &s) {
  float t = 1 - frac();
  enemyWarning(s, x, y, radius, 0.5f + 0.4f * t, t);
  s.color(tone.alpha(0.6f));
  s.ring(x, y, radius * t);
  s.color(col::white);
}

void Blast::renderLight(Screen &s) {
  s.glow(x, y - 4, 50 + 40 * (1 - frac()), tone * (0.25f + 0.35f * (1 - frac())));
}

void FrostPatch::tick() {
  Particle::tick();
  if (!hostile) {
    level->enemiesNear(x, y, 14, [&](Enemy &e) {
      double ex = e.x - x, ey = e.y - y;
      if (!e.type.floats && ex * ex + ey * ey < 12 * 12)
        e.afflict(StatusKind::Chill, 0.5f, 0.45f);
    });
    return;
  }
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  if (xd * xd + yd * yd < 10 * 10)
    p.afflict(StatusKind::Chill, 0.4f, 0.4f);
}

void FrostPatch::render(Screen &s) {
  float a = std::min(1.0f, frac() * 2) * 0.55f;
  s.rect(x - 4, y - 2, 8, 3, tone.alpha(a));
  s.rect(x - 2, y - 3, 4, 5, tone.alpha(a * 0.8f));
}

void Bramble::tick() {
  Particle::tick();
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  if (xd * xd + yd * yd < 11 * 11)
    p.afflict(StatusKind::Chill, 0.4f, 0.35f), p.afflict(StatusKind::Bleed, 1, 0.5f);
}

void Bramble::render(Screen &s) {
  float a = std::min(1.0f, frac() * 2);
  s.color(Color{0.55f, 0.7f, 0.45f, a});
  s.sprite(atlas::kit(atlas::kt::Thornbush), x, y + 2, 8, 16, 0.8f, 0.6f);
  s.color(col::white);
}
