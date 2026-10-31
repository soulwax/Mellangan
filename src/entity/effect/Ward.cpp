#include "entity/effect/Ward.hpp"

#include "entity/Mob.hpp"
#include "level/Level.hpp"

namespace {
constexpr Color SHIELD{1, 0.9f, 0.55f, 1};
}

Ward::Ward(const Mob &owner, float life)
    : owner(owner.id), cy(owner.bodyY()), life(life), maxLife(life) {
  x = owner.x, y = owner.y + 1;
}

float Ward::fade() const {
  return std::min({1.0f, life * 3, (maxLife - life) * 10});
}

void Ward::tick() {
  Mob *m = level->mob(owner);
  if (!m || m->hp <= 0 || (life -= DT) <= 0)
    return remove();
  x = m->x, y = m->y + 1, cy = m->bodyY();
  spin += DT * 4;
}

void Ward::render(Screen &s) {
  float a = fade(), beat = 0.85f + 0.15f * std::sin(spin * 3);
  s.color(SHIELD.alpha(0.16f * a));
  s.center(atlas::Glow, xp, cy, 34 / float(atlas::Glow.w));
  s.color(col::white);
  s.ring(xp, cy, 13, SHIELD.alpha(0.85f * a * beat));
  for (int i = 0; i < 4; i++) {
    float t = spin + TAU * i / 4;
    s.particle(atlas::pt::HolyGlint, xp + std::cos(t) * 14,
               cy + std::sin(t) * 14, SHIELD.alpha(0.8f * a), 0.75f);
  }
}

void Ward::renderLight(Screen &s) {
  s.glow(xp, cy, 60, Color{0.3f, 0.26f, 0.1f, 1} * fade());
}
