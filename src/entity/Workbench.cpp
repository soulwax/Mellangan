#include "entity/Workbench.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {

constexpr int TABLE = 72, ANVIL = 74;

} // namespace

void Workbench::onAdded() { level->tag(this); }

void Workbench::render(Screen &s) {
  if (level->below())
    return;
  s.sprite(atlas::tile(TABLE), xp, yp, 8, 16, 1, 1);
  s.sprite(atlas::tile(ANVIL), xp + 11, yp + 1, 8, 16, 0.75f, 0.75f);
}

void Workbench::renderShadow(Screen &s) {
  if (level->below())
    return;
  s.sprite(atlas::Shadow, xp + 3, yp + 1, 8, 4, 1.3f, 0.7f);
}

Workbench *Workbench::near(Level &level) {
  Workbench *b = level.get<Workbench>(level.vendor.bench);
  Player &p = *level.player;
  if (!b || p.dead() || level.below())
    return nullptr;
  double xd = b->x - p.x, yd = b->y - p.y;
  return xd * xd + yd * yd < REACH * REACH ? b : nullptr;
}
