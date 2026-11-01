#include "item/weapon/form/Forms.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "level/Level.hpp"

void BoltForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), c.base + (i - (c.count - 1) * 0.5) * 0.12, i);
  Sound::cast.play(0.2f);
}

float BoltForm::life(const RecipeShot &s) const {
  return s.reach / std::max(1.0f, s.r.speed);
}

void BoltForm::tick(RecipeShot &s, Mob &, float speed) const {
  s.x += s.dx * speed * DT, s.y += s.dy * speed * DT;
  s.level->foes(s.hostile(), s.x, s.y, s.radius + 24, [&](Mob &e) {
    if (s.removed || s.hitBefore(e.id))
      return;
    double ex = e.x - s.x, ey = e.bodyY() - s.y;
    float rr = s.radius + e.radius;
    if (ex * ex + ey * ey >= rr * rr)
      return;
    s.strike(e, s.dx, s.dy);
    if (--s.hitsLeft <= 0)
      s.remove();
  });
}

void BoltForm::render(RecipeShot &s, Screen &screen) const {
  glint(s, screen, float(std::atan2(s.dy, s.dx)));
}

void VolleyForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(),
           c.base + (c.count > 1 ? (i / double(c.count - 1) - 0.5) * 0.9 : 0), i);
  Sound::cast.play(0.2f);
}

void SeekerForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), c.base + (i - (c.count - 1) * 0.5) * 0.7, i);
  Sound::orb.play(0.2f);
}

void SeekerForm::tick(RecipeShot &s, Mob &p, float speed) const {
  s.steer(260);
  BoltForm::tick(s, p, speed);
}

void BoomerangForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), c.base + (i - (c.count - 1) * 0.5) * 0.35, i);
  Sound::axe.play(0.2f);
}

float BoomerangForm::life(const RecipeShot &s) const {
  return std::min(4.0f, 2.4f * s.reach / std::max(1.0f, s.r.speed));
}

void BoomerangForm::tick(RecipeShot &s, Mob &p, float speed) const {
  if (!s.back && s.age >= s.reach / std::max(1.0f, s.r.speed))
    s.back = true, s.hits.clear();
  if (s.back) {
    double hx = p.x - s.x, hy = p.bodyY() - s.y, d = std::hypot(hx, hy);
    if (d < 10)
      return s.remove();
    s.dx = hx / d, s.dy = hy / d;
  }
  float v = s.back ? speed * 1.25f : speed;
  s.x += s.dx * v * DT, s.y += s.dy * v * DT;
  s.touch(s.radius + 2);
}

void BoomerangForm::render(RecipeShot &s, Screen &screen) const {
  glint(s, screen, s.age * 14);
}
