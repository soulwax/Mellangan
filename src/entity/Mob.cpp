#include "entity/Mob.hpp"

#include <cstring>

#include "item/Upgrade.hpp"
#include "gfx/StatusVisual.hpp"

void Mob::drawBody(Screen &s, int px, int py, Sprite body, bool floats, float move,
                   Color tint, float flashAmount, Color flashColor) const {
  float sx = 1, sy = 1, rot = 0;
  if (!floats) {
    float w = std::sin(animT * 14);
    sy = 1 + (0.025f + 0.06f * move) * w;
    sx = 1 - (0.02f + 0.04f * move) * w;
    rot = std::sin(animT * 7) * 0.08f * move;
  }
  s.color(tint);
  if (status.power(StatusKind::Burn) > 0)
    s.color({tint.r, tint.g * 0.65f, tint.b * 0.45f, tint.a});
  else if (status.power(StatusKind::Poison) > 0)
    s.color({tint.r * 0.65f, tint.g, tint.b * 0.6f, tint.a});
  else if (status.power(StatusKind::Chill) > 0)
    s.color({tint.r * 0.65f, tint.g * 0.85f, tint.b, tint.a});
  else if (status.power(StatusKind::Marked) > 0)
    s.color({tint.r, tint.g * 0.65f, tint.b, tint.a});
  s.flash(body, px, py, 8, 16, sx * scale * facing, sy * scale,
          rot, flashAmount, flashColor);
  s.color(col::white);
  statusIcons(s, status, px, py - 18 * scale);
}

void Mob::drawShadow(Screen &s, int px, int py, bool floats) const {
  float k = scale * (floats ? 0.7f : 1.0f);
  s.sprite(atlas::Shadow, px, py + 1, 8, 4, k, k);
}

Mob::Mob() = default;
Mob::~Mob() = default;

void Mob::tickKit() {
  for (auto &up : upgrades) {
    up->holder = this;
    if (up->level > 0)
      up->tick(*this);
  }
}

Upgrade *Mob::grant(const char *name, int levels) {
  if (upgrades.empty())
    upgrades = Upgrade::all();
  for (auto &up : upgrades)
    if (std::strcmp(up->name, name) == 0) {
      up->holder = this, up->foe = hostile();
      for (int i = 0; i < levels && !up->maxed(); i++)
        up->levelUp(*this);
      return up.get();
    }
  return nullptr;
}
