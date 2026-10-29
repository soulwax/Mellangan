#include "particle/Mote.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

Mote::Mote(double x, double y, int icon, Color color, float life)
    : Particle(x, y, life), icon(icon), color(color), x0(x), y0(y),
      bend(rng.range(-28, 28)) {}

void Mote::tick() {
  Particle::tick();
  const Player &p = *level->player;
  double k = 1 - frac(), e = k * k;
  double tx = p.x, ty = p.bodyY(), dx = tx - x0, dy = ty - y0;
  double len = std::max(1.0, std::sqrt(dx * dx + dy * dy)),
         arc = std::sin(k * PI) * bend;
  double nx = x0 + dx * e - dy / len * arc, ny = y0 + dy * e + dx / len * arc;
  if (std::abs(nx - x) + std::abs(ny - y) > 0.05)
    heading = float(std::atan2(-(nx - x), ny - y));
  x = nx, y = ny;
}

void Mote::render(Screen &s) {
  float a = std::min(1.0f, (maxLife - life) * 8);
  s.color(color.alpha(0.35f * a));
  s.center(atlas::Glow, xp, yp, 14 / float(atlas::Glow.w));
  s.color(col::white);
  s.particle(icon, xp, yp, color.alpha(color.a * a), 1, heading);
}

void Mote::renderLight(Screen &s) { s.glow(xp, yp, 24, color * 0.25f); }
