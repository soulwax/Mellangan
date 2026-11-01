#include "item/weapon/form/Forms.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "level/Level.hpp"

namespace {

constexpr float CRACK = 0.08f, WALL_GAP = 48, WALL_STEP = 22, WALL_THICK = 6;
constexpr float SPIN = 3.0f, SPREAD = 0.8f;

void strikeHarder(RecipeShot &s, Mob &e, float k) {
  float d = s.damage;
  s.damage *= k;
  s.strike(e, s.dx, s.dy);
  s.damage = d;
}

} // namespace

void WhipForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), c.base + (i % 2 ? -0.25 : 0.25) * (i > 0), i)->age =
        -0.15f * i;
  Sound::slash.play(0.25f, 0.05f, 1.2f);
}

void WhipForm::tick(RecipeShot &s, Mob &p, float) const {
  s.ox = p.x, s.oy = p.bodyY();
  if (s.age < CRACK || s.pulse != 0)
    return;
  s.pulse = 1;
  s.level->foes(s.hostile(), s.ox + s.dx * s.reach / 2, s.oy + s.dy * s.reach / 2,
                s.reach / 2 + 24, [&](Mob &e) {
                  double ex = e.x - s.ox, ey = e.bodyY() - s.oy;
                  double b = std::clamp(ex * s.dx + ey * s.dy, 0.0, double(s.reach));
                  double qx = ex - s.dx * b, qy = ey - s.dy * b;
                  float rr = 8 + e.radius;
                  if (qx * qx + qy * qy < rr * rr && !s.hitBefore(e.id))
                    strikeHarder(s, e, b > s.reach * 0.75 ? 1.5f : 1.0f);
                });
}

void WhipForm::render(RecipeShot &s, Screen &screen) const {
  if (s.age < 0)
    return;
  Color c = s.r.tint;
  float k = std::min(1.0f, s.age / CRACK), fade = 1 - s.age / s.life;
  double nx = -s.dy, ny = s.dx, px = s.ox, py = s.oy;
  for (int j = 1; j <= 8; j++) {
    float t = j / 8.0f, sway = std::sin(t * 3.14159f) * (1 - k) * 14 * (s.index % 2 ? -1 : 1);
    double x = s.ox + s.dx * s.reach * t * k + nx * sway,
           y = s.oy + s.dy * s.reach * t * k + ny * sway;
    screen.line(px, py, x, y, 2, c.alpha(0.85f * fade));
    px = x, py = y;
  }
  if (k >= 1)
    screen.particle(s.r.particle, px, py, c.alpha(fade), 1.2f, s.age * 12);
}

void WallForm::fire(const FormCast &c) const {
  for (int i = 0; i < c.count; i++) {
    double d = WALL_GAP + WALL_STEP * i;
    c.shot(c.p.x + c.dx * d, c.p.bodyY() + c.dy * d, c.base, i);
  }
  Sound::cast.play(0.25f, 0.05f, 0.7f);
}

void WallForm::tick(RecipeShot &s, Mob &, float) const {
  if ((s.pulse -= DT) <= 0)
    s.pulse = 0.5f, s.hits.clear();
  double tx = -s.dy, ty = s.dx, half = s.reach / 2;
  s.level->foes(s.hostile(), s.x, s.y, float(half) + 24, [&](Mob &e) {
    double ex = e.x - s.x, ey = e.bodyY() - s.y;
    double along = ex * tx + ey * ty, across = ex * s.dx + ey * s.dy;
    double edge = WALL_THICK + e.radius;
    if (std::abs(along) > half + e.radius || std::abs(across) > edge)
      return;
    double push = (across < 0 ? -edge : edge) - across;
    e.x += s.dx * push, e.y += s.dy * push;
    if (!s.hitBefore(e.id))
      s.strike(e, across < 0 ? -s.dx : s.dx, across < 0 ? -s.dy : s.dy);
  });
}

void WallForm::render(RecipeShot &s, Screen &screen) const {
  Color c = s.r.tint;
  float fade = std::min(1.0f, (s.life - s.age) * 2) * std::min(1.0f, s.age * 6);
  double tx = -s.dy, ty = s.dx, half = s.reach / 2;
  screen.line(s.x - tx * half, s.y - ty * half, s.x + tx * half, s.y + ty * half, 3,
              c.alpha(0.45f * fade));
  int n = std::max(3, int(s.reach / 10));
  for (int j = 0; j <= n; j++) {
    double t = (j / double(n) - 0.5) * 2 * half;
    screen.particle(s.r.particle, s.x + tx * t, s.y + ty * t - 3, c.alpha(fade), 1,
                    float(j) + s.age);
  }
}

void WallForm::light(RecipeShot &s, Screen &screen) const {
  screen.glow(s.x, s.y, s.reach * 0.8f, s.r.tint * 0.08f);
}

void SpiralForm::fire(const FormCast &c) const {
  double lead = SPIN * c.reach * 0.5 / std::max(1.0f, c.r.speed * SPREAD);
  for (int i = 0; i < c.count; i++)
    c.shot(c.p.x, c.p.bodyY(), c.base - lead + TAU * i / std::max(1, c.count), i);
  Sound::orb.play(0.2f, 0.05f, 1.1f);
}

float SpiralForm::life(const RecipeShot &s) const {
  return s.reach / std::max(1.0f, s.r.speed * SPREAD);
}

void SpiralForm::tick(RecipeShot &s, Mob &, float speed) const {
  if (s.age <= DT * 1.5f)
    s.tx = std::atan2(s.dy, s.dx);
  double a = s.tx + s.age * SPIN, r = speed * SPREAD * s.age;
  double x = s.ox + std::cos(a) * r, y = s.oy + std::sin(a) * r;
  s.dx = x - s.x, s.dy = y - s.y;
  unit(s.dx, s.dy);
  s.x = x, s.y = y;
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
