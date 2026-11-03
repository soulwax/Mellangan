#include "campaign/Sentry.hpp"

#include <cmath>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color STEEL{0.85f, 0.88f, 0.95f, 1};
constexpr Color DECK{0.56f, 0.38f, 0.22f, 1}, DECK_DARK{0.34f, 0.22f, 0.13f, 1},
                RAIL{0.42f, 0.28f, 0.16f, 1};

} // namespace

void Sentry::onAdded() { level->tag(this); }

Enemy *Sentry::target() const {
  Enemy *best = nullptr;
  double bestD = GUARD * GUARD;
  level->enemiesNear(post.x, post.y, GUARD, [&](Enemy &e) {
    if (e.removed || e.buried || e.hp <= 0)
      return;
    double xd = e.x - post.x, yd = e.y - post.y, d2 = xd * xd + yd * yd;
    if (d2 < bestD)
      bestD = d2, best = &e;
  });
  return best;
}

void Sentry::shove() {
  level->enemiesNear(x, y, BODY + 16, [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y, d2 = xd * xd + yd * yd, min = BODY + e.radius;
    if (e.removed || d2 >= min * min || d2 < 1e-6)
      return;
    double d = std::sqrt(d2);
    e.x = x + xd / d * min, e.y = y + yd / d * min;
  });
}

float Sentry::might() const {
  const Balance &b = level->game.balance;
  float t = level->rules.threat ? level->rules.threat(post.x, post.y) : level->time;
  return DAMAGE * b.enemyHp * (1 + t / b.enemyHpRamp);
}

void Sentry::cleave() {
  float hit = might();
  level->enemiesNear(x, y, CLEAVE + 16, [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y, r = CLEAVE + e.radius;
    if (e.removed || e.buried || e.hp <= 0 || xd * xd + yd * yd > r * r)
      return;
    e.garrison = true;
    e.hurt(hit, xd, yd, KNOCK);
    Spark::burst(*level, e.x, e.bodyY(), STEEL, 3, 50, false);
    if (e.removed || e.hp <= 0)
      kills++;
  });
  Sound::hurt.play(0.08f, 0.1f, 1.6f);
}

void Sentry::tick() {
  t += DT, swing -= DT, lunge = std::max(0.0f, lunge - DT * 4);
  shove();
  double tx = post.x, ty = post.y;
  Enemy *e = target();
  if (e) {
    double xd = e->x - x, yd = e->y - y, d = std::sqrt(xd * xd + yd * yd);
    facing = xd < 0 ? -1.0f : 1.0f;
    if (d <= REACH + e->radius) {
      if (swing <= 0) {
        swing = SWING, lunge = 1;
        cleave();
      }
      return;
    }
    tx = e->x, ty = e->y;
  }
  double xd = tx - x, yd = ty - y, d = std::sqrt(xd * xd + yd * yd);
  if (d < 1)
    return;
  double step = std::min(d, double(SPEED * DT));
  double nx = x + xd / d * step, ny = y + yd / d * step;
  double px = nx - post.x, py = ny - post.y, pd = std::sqrt(px * px + py * py);
  if (pd > GUARD * 0.7)
    nx = post.x + px / pd * GUARD * 0.7, ny = post.y + py / pd * GUARD * 0.7;
  x = nx, y = ny;
  if (!e)
    facing = xd < 0 ? -1.0f : 1.0f;
}

void Sentry::render(Screen &s) {
  float jab = lunge * 3 * facing, bob = std::abs(std::sin(t * 6)) * 0.6f;
  s.sprite(atlas::tile(tile), xp + jab, yp - bob, 8, 16, facing, 1);
  if (lunge > 0)
    s.line(xp + facing * 6, yp - 9, xp + facing * (12 + 6 * lunge), yp - 11, 1, STEEL.alpha(lunge));
}

void Sentry::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 1, 0.7f);
}

void Bridge::render(Screen &s) {
  float w = across ? 22 : 48, h = across ? 48 : 22, x0 = xp - w / 2, y0 = yp - 5 - h / 2;
  s.rect(x0, y0, w, h, DECK_DARK);
  s.rect(x0 + 1, y0 + 1, w - 2, h - 2, DECK);
  for (int i = 3; i < (across ? w : h) - 1; i += 4)
    if (across)
      s.rect(x0 + i, y0 + 1, 1, h - 2, DECK_DARK);
    else
      s.rect(x0 + 1, y0 + i, w - 2, 1, DECK_DARK);
  if (across)
    s.rect(x0, y0, w, 2, RAIL), s.rect(x0, y0 + h - 2, w, 2, RAIL);
  else
    s.rect(x0, y0, 2, h, RAIL), s.rect(x0 + w - 2, y0, 2, h, RAIL);
}
