#include "item/weapon/form/Forms.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "gfx/Screen.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Mark.hpp"

void OrbitForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), 0, i);
  Sound::orb.play(0.25f);
}

void OrbitForm::tick(RecipeShot &s, Mob &p, float) const {
  double a = s.age * s.r.speed + TAU * s.index / s.of;
  s.x = p.x + std::cos(a) * s.reach, s.y = p.bodyY() + std::sin(a) * s.reach * 0.8;
  s.dx = -std::sin(a), s.dy = std::cos(a);
  if ((s.pulse -= DT) <= 0)
    s.pulse = 0.6f, s.hits.clear();
  s.level->foes(s.hostile(), s.x, s.y, s.radius + 24, [&](Mob &e) {
    if (s.hitBefore(e.id))
      return;
    double ex = e.x - s.x, ey = e.bodyY() - s.y;
    float rr = s.radius + e.radius;
    if (ex * ex + ey * ey < rr * rr)
      s.strike(e, ex, ey);
  });
}

void OrbitForm::render(RecipeShot &s, Screen &screen) const {
  glint(s, screen, s.age * 6);
}

void NovaForm::fire(const FormCast &c) const {
  c.shot(c.p.x, c.p.bodyY(), 0, 0);
  Sound::nova.play(0.3f);
}

void NovaForm::tick(RecipeShot &s, Mob &, float) const {
  float ring = s.reach * std::min(1.0f, s.age / 0.35f);
  s.level->foes(s.hostile(), s.x, s.y, ring + 16, [&](Mob &e) {
    if (s.hitBefore(e.id))
      return;
    double ex = e.x - s.x, ey = e.bodyY() - s.y, d = std::hypot(ex, ey);
    if (d < ring + e.radius)
      s.strike(e, ex, ey);
  });
}

void NovaForm::render(RecipeShot &s, Screen &screen) const {
  Color c = s.r.tint;
  float k = std::min(1.0f, s.age / 0.35f), ring = s.reach * k,
        a = 1 - std::max(0.0f, (s.age - 0.25f) / 0.2f);
  screen.ring(s.xp, s.yp, ring, c.alpha(0.8f * a), 2);
  screen.ring(s.xp, s.yp, ring * 0.8f, c.alpha(0.35f * a));
  for (int i = 0; i < 10; i++) {
    float t = TAU * i / 10 + s.age * 2;
    screen.particle(s.r.particle, s.xp + std::cos(t) * ring,
                    s.yp + std::sin(t) * ring, c.alpha(a), 0.8f, t);
  }
}

void NovaForm::light(RecipeShot &s, Screen &screen) const {
  screen.glow(s.xp, s.yp, s.reach * 1.5f, s.r.tint * 0.12f);
}

void RainForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++) {
    double x, y;
    if (!c.targets.empty()) {
      Mob *e = c.targets[size_t(i) % c.targets.size()];
      x = e->x, y = e->bodyY();
    } else {
      float a = rng.range(0, TAU), k = rng.range(0.3f, 1.0f) * c.reach;
      x = c.p.x + std::cos(a) * k, y = c.p.bodyY() + std::sin(a) * k;
    }
    c.shot(x + rng.range(-8, 8), y + rng.range(-8, 8), 0, i);
  }
  Sound::cast.play(0.25f);
}

void RainForm::tick(RecipeShot &s, Mob &, float) const {
  if (s.age >= 0.5f && s.pulse == 0) {
    s.pulse = 1;
    s.blast(26, s.damage);
    s.level->add<Mark>(s.x, s.y, 1.5f, atlas::Glow, s.r.tint.alpha(0.12f),
                       60.0f / atlas::Glow.w);
  }
}

void RainForm::render(RecipeShot &s, Screen &screen) const {
  Color c = s.r.tint;
  float k = std::min(1.0f, s.age / 0.5f);
  if (k < 1) {
    screen.ring(s.xp, s.yp, 26 * (1.6f - 0.6f * k), c.alpha(0.25f + 0.5f * k));
    screen.particle(s.r.particle, s.xp + 30 * (1 - k), s.yp - 140 * (1 - k * k),
                    c, 1.4f, 2.4f);
  }
}

void RainForm::light(RecipeShot &s, Screen &screen) const {
  if (s.pulse <= 0)
    FormBehavior::light(s, screen);
}

void FieldForm::fire(const FormCast &c) const {
  double x = c.p.x + c.dx * 60, y = c.p.bodyY() + c.dy * 60;
  if (!c.targets.empty())
    x = c.targets.front()->x, y = c.targets.front()->bodyY();
  c.shot(x, y, 0, 0);
  Sound::orb.play(0.3f);
}

void FieldForm::tick(RecipeShot &s, Mob &, float) const {
  if ((s.pulse -= DT) > 0)
    return;
  s.pulse = 0.5f;
  s.hits.clear();
  s.level->foes(s.hostile(), s.x, s.y, s.reach + 16, [&](Mob &e) {
    double ex = e.x - s.x, ey = e.bodyY() - s.y;
    float rr = s.reach + e.radius;
    if (ex * ex + ey * ey < rr * rr)
      s.strike(e, ex, ey);
  });
}

void FieldForm::render(RecipeShot &s, Screen &screen) const {
  Color c = s.r.tint;
  float fade = std::min({1.0f, s.age * 4, (s.life - s.age) * 3});
  screen.ring(s.xp, s.yp, s.reach, c.alpha(0.6f * fade));
  screen.ring(s.xp, s.yp, s.reach * 0.6f, c.alpha(0.3f * fade));
  for (int i = 0; i < 6; i++) {
    float t = -s.age * 2.2f + TAU * i / 6;
    screen.particle(s.r.particle, s.xp + std::cos(t) * s.reach * 0.8f,
                    s.yp + std::sin(t) * s.reach * 0.8f, c.alpha(0.8f * fade), 1, t);
  }
  screen.color(c.alpha(0.3f * fade));
  screen.center(atlas::Glow, s.xp, s.yp,
                s.reach * (1.1f + 0.1f * std::sin(s.age * 8)) / atlas::Glow.w);
  screen.color(col::white);
}

void FieldForm::light(RecipeShot &s, Screen &screen) const {
  screen.glow(s.xp, s.yp, s.reach * 2, s.r.tint * 0.12f);
}

void FieldForm::trail(RecipeShot &s) const {
  if (!rng.chance(0.5f) || !Drift::room())
    return;
  float a = rng.range(0, TAU);
  Drift *d = s.level->add<Drift>(s.x + std::cos(a) * s.reach, s.y + std::sin(a) * s.reach,
                                 -std::cos(a) * s.reach * 1.6f,
                                 -std::sin(a) * s.reach * 1.6f, s.r.particle,
                                 s.r.tint.alpha(0.7f), 0.5f);
  d->scale = 0.6f, d->spin = 3, d->drag = 0;
}
