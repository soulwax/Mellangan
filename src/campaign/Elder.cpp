#include "campaign/Elder.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {

constexpr int ELDER = 100;

} // namespace

void Elder::onAdded() {
  level->tag(this);
  level->collide(x, y, 5);
}

void Elder::tick() {
  t += DT;
  Player &p = *level->player;
  if (std::abs(p.x - x) > 2)
    facing = p.x < x ? -1.0f : 1.0f;
}

void Elder::render(Screen &s) {
  float bob = 1 + 0.025f * std::sin(t * 1.8f);
  s.sprite(atlas::tile(ELDER), xp, yp, 8, 16, facing, bob);
}

void Elder::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 0.9f, 0.7f);
}

Elder *Elder::near(Level &level) {
  if (!level.game.campaign)
    return nullptr;
  Elder *e = level.get<Elder>(level.game.campaign->elder);
  Player &p = *level.player;
  if (!e || p.dead())
    return nullptr;
  double xd = e->x - p.x, yd = e->y - p.y;
  return xd * xd + yd * yd < REACH * REACH ? e : nullptr;
}
