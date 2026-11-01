#include "hero/Corsair.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Flourish.hpp"

namespace {
bool streaker(const Codex &c) { return c.bestStreak >= 30; }
} // namespace

Corsair::Corsair()
    : ExpansionHero({"CORSAIR", "Harpoon", "Rover of the moonless coast",
                     "Critical hits deal 220% damage", "reach a 30-kill streak", streaker, 84,
                     {0.4f, 0.85f, 0.9f, 1}, -10, 1.12f, 0, 0, 7, 0, 0, 0.06f,
                     "BROADSIDE", "Dash forward and fire five piercing shots", 8,
                     {"Harpoon", "Boomerang", "Quicksilver"}},
                    3) {}

void Corsair::cast(Mob &p) const {
  if (p.dead())
    return;
  double dir = std::atan2(p.yAim, p.xAim);
  for (int i = -2; i <= 2; i++) {
    double a = dir + i * 0.16;
    p.level->add<Arrow>(p.x, p.bodyY(), std::cos(a) * 270, std::sin(a) * 270,
                        24 * p.might, 3, ArrowStyle::Piercing)->hostile = p.hostile();
  }
  double x0 = p.x, y0 = p.bodyY();
  flourish::echo(*p.level, p.x, p.y, sprite(), Color{0.4f, 0.85f, 1, 1});
  for (int i = 0; i < 8; i++) {
    p.x += std::cos(dir) * 4;
    p.y += std::sin(dir) * 4;
    p.level->collide(p.x, p.y, p.radius);
  }
  p.shield(0.6f);
  flourish::streak(*p.level, x0, y0, p.x, p.bodyY(), atlas::pt::Puff,
                   Color{0.85f, 0.95f, 1, 0.6f}, 5);
  for (int i = 0; i < 6 && Drift::room(); i++) {
    Drift *d = p.level->add<Drift>(x0, y0, -std::cos(dir) * 60 + rng.range(-30, 30),
                                   -std::sin(dir) * 60 - rng.range(20, 50),
                                   atlas::pt::FrostGlint, Color{0.4f, 0.85f, 1, 1}, 0.45f);
    d->gravity = 200, d->drag = 0.5f, d->scale = 0.6f;
  }
  Sound::axe.play(0.5f);
}
