#include "entity/projectile/Bomb.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "particle/Explosion.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Chip.hpp"

namespace {
constexpr Color FIRE{1, 0.55f, 0.2f, 1}, ACID{0.55f, 0.95f, 0.3f, 1};
}

void Bomb::tick() {
  t += DT / 0.55f;
  float k = std::min(t, 1.0f);
  x = x0 + (x1 - x0) * k;
  y = y0 + (y1 - y0) * k;
  z = std::sin(k * PI) * 40;
  if (t >= 1)
    return burst();
  bool acid = style == PoolStyle::Acid;
  if (rng.chance(acid ? 0.35f : 0.7f) && Spark::alive < 550)
    level->add<Spark>(x, y - z - (acid ? 4 : 10), rng.range(-6, 6),
                      acid ? rng.range(10, 25) : -rng.range(10, 25),
                      acid ? ACID : Color{1, 0.8f, 0.35f, 1}, 0.3f, 0.4f,
                      false);
}

void Bomb::burst() {
  if (hostile || (source && source->hostile())) {
    Player &p = *level->player;
    double xd = p.x - x, yd = p.y - y;
    float r = 28 + p.radius;
    if (!p.dead() && xd * xd + yd * yd < r * r)
      p.felledBy = "a bomb", p.strike(damage, xd, yd, 90);
  } else
    level->enemiesNear(x, y, 28 + 24, [&](Enemy &e) {
      double xd = e.x - x, yd = e.y - y;
      float r = 28 + e.radius;
      if (xd * xd + yd * yd < r * r)
        e.hurt(damage, xd, yd, 90, source);
    });
  FirePool *pool = level->add<FirePool>(x, y, poolRadius, poolDamage, poolTime, style);
  pool->source = source, pool->hostile = hostile || (source && source->hostile());
  if (style == PoolStyle::Acid) {
    level->add<Shockwave>(x, y, 3, poolRadius * 1.2f, 0.3f, ACID.alpha(0.8f),
                          false);
    for (int i = 0; i < 6 && Debris::room(); i++)
      level->add<Chip>(x, y, 4, atlas::pt::IceShard,
                       Color{0.8f, 1, 0.75f, 0.9f}, 1.1);
    Spark::burst(*level, x, y, ACID, 8, 90, true);
    Sound::explosion.play(0.25f, 0.2f);
  } else {
    level->add<Explosion>(x, y, 0);
    level->add<Shockwave>(x, y, 4, 28 * 1.3f, 0.3f, FIRE.alpha(0.85f));
    level->add<Mark>(x, y, poolTime + 1, atlas::Glow,
                     Color{0.06f, 0.04f, 0.03f, 0.45f}, 56 / float(atlas::Glow.w));
    Spark::burst(*level, x, y, FIRE, 6, 110, true);
    Sound::explosion.play(0.5f);
    level->game.onExplosion();
  }
  remove();
}

void Bomb::render(Screen &s) {
  s.sprite(atlas::Shadow, xp, std::floor(y) + 1, 8, 4, 0.6f, 0.6f);
  if (style == PoolStyle::Acid)
    return s.center(atlas::tile(127), xp, yp - 6, 1, t * 7);
  s.center(atlas::town(atlas::tt::Bomb), xp, yp - 6, 1, t * 9);
  s.particle(atlas::pt::FrostGlint, xp + std::sin(t * 9) * 4,
             yp - 12 - std::cos(t * 9) * 2, Color{1, 0.85f, 0.4f, 1},
             0.6f + 0.3f * std::sin(t * 40));
}
