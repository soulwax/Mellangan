#include "entity/effect/GlaiveThrust.hpp"

#include <algorithm>
#include <cmath>

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

GlaiveThrust::GlaiveThrust(const Mob &p, double dirX, double dirY,
                           float reach, float damage, float force,
                           bool doubleEnded)
    : owner(p), ownerId(p.id), dx(dirX), dy(dirY), reach(reach), damage(damage), force(force),
      doubleEnded(doubleEnded) {
  double len = std::sqrt(dx * dx + dy * dy);
  if (len > 1e-4)
    dx /= len, dy /= len;
  else
    dx = p.facing > 0 ? 1 : -1, dy = 0;
  x = p.x + dx * 10;
  y = p.bodyY() + dy * 10;
}

void GlaiveThrust::tick() {
  if (!level->mob(ownerId))
    return remove();
  t += DT / 0.20f;
  float ext = std::sin(std::min(t, 1.0f) * PI) * reach;
  x = owner.x + dx * ext;
  y = owner.bodyY() + dy * ext;

  auto checkLine = [&](double sx, double sy, double ex, double ey) {
    level->foes((source && source->hostile()), (sx + ex) * 0.5, (sy + ey) * 0.5, reach * 0.6f + 16, [&](Mob &e) {
      if (std::find(hits.begin(), hits.end(), e.id) != hits.end())
        return;
      double vx = ex - sx, vy = ey - sy;
      double wx = e.x - sx, wy = e.bodyY() - sy;
      double c1 = wx * vx + wy * vy;
      double c2 = vx * vx + vy * vy;
      double b = c2 > 1e-4 ? std::clamp(c1 / c2, 0.0, 1.0) : 0.0;
      double projX = sx + b * vx, projY = sy + b * vy;
      double dist2 = (e.x - projX) * (e.x - projX) + (e.bodyY() - projY) * (e.bodyY() - projY);
      float r = 10 + e.radius;
      if (dist2 <= r * r) {
        hits.push_back(e.id);
        e.strike(damage, vx, vy, force, source);
      }
    });
  };

  checkLine(owner.x, owner.bodyY(), owner.x + dx * ext, owner.bodyY() + dy * ext);
  if (doubleEnded)
    checkLine(owner.x, owner.bodyY(), owner.x - dx * ext * 0.85, owner.bodyY() - dy * ext * 0.85);

  if (t >= 1)
    remove();
}

void GlaiveThrust::render(Screen &s) {
  float ext = std::sin(std::min(t, 1.0f) * PI) * reach;
  float rot = float(std::atan2(dy, dx)) + PI * 0.25f;
  float a = 0.45f * (1 - std::min(t, 1.0f));
  double by = owner.bodyY();
  for (int i = -1; i <= 1; i++) {
    double ox = -dy * 4 * i, oy = dx * 4 * i, from = 8 + 6 * std::abs(i);
    s.line(owner.x + dx * from + ox, by + dy * from + oy,
           owner.x + dx * (ext - 4) + ox, by + dy * (ext - 4) + oy,
           i == 0 ? 2.0f : 1.0f, Color{1, 0.95f, 0.8f, i == 0 ? a : a * 0.6f});
  }
  s.center(atlas::frontier(atlas::fr::Glaive), owner.x + dx * ext,
           owner.bodyY() + dy * ext, 1.25f, rot);
  if (doubleEnded) {
    s.center(atlas::frontier(atlas::fr::Glaive), owner.x - dx * ext * 0.85,
             owner.bodyY() - dy * ext * 0.85, 1.15f, rot + PI);
  }
}

void GlaiveThrust::renderLight(Screen &s) {
  s.glow(xp, yp, 45, {0.25f, 0.2f, 0.1f, 1});
  if (doubleEnded) {
    float ext = std::sin(std::min(t, 1.0f) * PI) * reach;
    s.glow(owner.x - dx * ext * 0.85, owner.bodyY() - dy * ext * 0.85, 38,
           {0.25f, 0.2f, 0.1f, 1});
  }
}

