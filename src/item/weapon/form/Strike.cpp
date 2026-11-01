#include "item/weapon/form/Forms.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "gfx/Screen.hpp"
#include "level/Level.hpp"

void CleaveForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), c.base, i)->age = -0.16f * i;
  Sound::slash.play(0.25f);
}

void CleaveForm::tick(RecipeShot &s, Mob &p, float) const {
  s.x = p.x, s.y = p.bodyY();
  if (s.age < 0)
    return;
  s.level->foes(s.hostile(), s.x, s.y, s.reach + 24, [&](Mob &e) {
    if (s.hitBefore(e.id))
      return;
    double ex = e.x - s.x, ey = e.bodyY() - s.y, d = std::hypot(ex, ey);
    if (d < s.reach + e.radius && ex * s.dx + ey * s.dy > d * 0.3)
      s.strike(e, ex, ey);
  });
}

void CleaveForm::render(RecipeShot &s, Screen &screen) const {
  if (s.age < 0)
    return;
  Color c = s.r.tint;
  float k = s.age / s.life, side = s.index % 2 ? -1.0f : 1.0f,
        a0 = float(std::atan2(s.dy, s.dx));
  auto at = [&](float t) { return a0 + side * (1.2f - 2.4f * t); };
  for (int j = 0; j < 6; j++) {
    float a1 = at(k * j / 6), a2 = at(k * (j + 1) / 6);
    screen.line(s.x + std::cos(a1) * s.reach, s.y + std::sin(a1) * s.reach,
                s.x + std::cos(a2) * s.reach, s.y + std::sin(a2) * s.reach, 2,
                c.alpha(0.6f * (1 - k) * (j + 1) / 6));
  }
  for (int j = 0; j < 4; j++) {
    float t = k - j * 0.07f;
    if (t < 0)
      break;
    float a = at(t), rr = s.reach * 0.75f;
    screen.particle(s.r.particle, s.x + std::cos(a) * rr, s.y + std::sin(a) * rr,
                    c.alpha((1 - j / 4.0f) * (1 - k * 0.5f)), 1.2f - j * 0.15f,
                    a + 1.57f);
  }
}

void CleaveForm::light(RecipeShot &s, Screen &screen) const {
  if (s.age >= 0)
    FormBehavior::light(s, screen);
}

void LobForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++) {
    double x = c.p.x + c.dx * c.reach * 0.6, y = c.p.bodyY() + c.dy * c.reach * 0.6;
    if (!c.targets.empty()) {
      Mob *e = c.targets[size_t(i) % c.targets.size()];
      x = e->x, y = e->bodyY();
    }
    RecipeShot *s = c.shot(c.p.x, c.p.bodyY(), c.base, i);
    s->tx = x + (i ? rng.range(-14, 14) : 0);
    s->ty = y + (i ? rng.range(-10, 10) : 0);
  }
  Sound::axe.play(0.2f, 0.06f, 0.8f);
}

void LobForm::tick(RecipeShot &s, Mob &, float) const {
  float k = std::min(1.0f, s.age / s.life);
  s.x = s.ox + (s.tx - s.ox) * k, s.y = s.oy + (s.ty - s.oy) * k;
}

void LobForm::render(RecipeShot &s, Screen &screen) const {
  Color c = s.r.tint;
  float k = std::min(1.0f, s.age / s.life);
  screen.ring(s.tx, s.ty, 10 + 18 * (1 - k), c.alpha(0.2f + 0.4f * k));
  screen.particle(s.r.particle, s.x, s.y - std::sin(k * 3.14159f) * 38, c, 1.1f,
                  s.age * 9);
}

void LobForm::expire(RecipeShot &s) const {
  s.x = s.tx, s.y = s.ty, s.blast(28, s.damage);
}

void TrapForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++) {
    float a = rng.range(0, TAU), k = i ? rng.range(14, 30) : 0;
    c.shot(c.p.x + std::cos(a) * k, c.p.y + std::sin(a) * k * 0.7f, 0, i);
  }
  Sound::axe.play(0.15f, 0.06f, 1.4f);
}

void TrapForm::tick(RecipeShot &s, Mob &, float) const {
  if (s.age < 0.6f)
    return;
  bool sprung = false;
  s.level->foes(s.hostile(), s.x, s.y, 40, [&](Mob &e) {
    double ex = e.x - s.x, ey = e.y - s.y;
    float rr = 10 + e.radius;
    sprung |= ex * ex + ey * ey < rr * rr;
  });
  if (sprung) {
    s.blast(s.reach, s.damage);
    s.remove();
  }
}

void TrapForm::render(RecipeShot &s, Screen &screen) const {
  Color c = s.r.tint;
  float fade = std::min({1.0f, s.age * 4, (s.life - s.age) * 2});
  screen.particle(s.r.particle, s.x, s.y - 2, c.alpha(fade), 0.9f, 0);
  if (s.age > 0.6f)
    screen.ring(s.x, s.y, 8 + 2 * std::sin(s.age * 6), c.alpha(0.35f * fade));
}

void TrapForm::light(RecipeShot &s, Screen &screen) const {
  screen.glow(s.xp, s.yp, 16, s.r.tint * 0.12f);
}

void BeamForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), c.base + (i - (c.count - 1) * 0.5) * 0.18, i);
  Sound::cast.play(0.25f, 0.06f, 1.3f);
}

void BeamForm::tick(RecipeShot &s, Mob &, float) const {
  if (s.pulse != 0)
    return;
  s.pulse = 1;
  s.tx = s.ox + s.dx * s.reach, s.ty = s.oy + s.dy * s.reach;
  s.level->foes(s.hostile(), (s.ox + s.tx) / 2, (s.oy + s.ty) / 2,
                s.reach / 2 + 24, [&](Mob &e) {
                  double ex = e.x - s.ox, ey = e.bodyY() - s.oy;
                  double b = std::clamp(ex * s.dx + ey * s.dy, 0.0, double(s.reach));
                  double qx = ex - s.dx * b, qy = ey - s.dy * b;
                  float rr = 7 + e.radius;
                  if (qx * qx + qy * qy < rr * rr)
                    s.strike(e, s.dx, s.dy);
                });
}

void BeamForm::render(RecipeShot &s, Screen &screen) const {
  if (s.pulse == 0)
    return;
  Color c = s.r.tint;
  float a = 1 - s.age / s.life;
  screen.line(s.ox, s.oy, s.tx, s.ty, 1 + 3 * a, c.alpha(0.7f * a));
  screen.line(s.ox, s.oy, s.tx, s.ty, 1, col::white.alpha(a));
  for (int j = 1; j <= 5; j++) {
    float t = j / 5.0f;
    screen.particle(s.r.particle, s.ox + (s.tx - s.ox) * t,
                    s.oy + (s.ty - s.oy) * t, c.alpha(a), 0.7f, s.age * 4 + j);
  }
}

void BeamForm::light(RecipeShot &s, Screen &screen) const {
  if (s.pulse != 0)
    screen.glow((s.ox + s.tx) / 2, (s.oy + s.ty) / 2, s.reach, s.r.tint * 0.1f);
}
