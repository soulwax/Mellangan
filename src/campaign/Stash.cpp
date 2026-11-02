#include "campaign/Stash.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

void Stash::onAdded() {
  level->tag(this);
  level->collide(x, y, 6);
}

void Stash::render(Screen &s) {
  s.sprite(atlas::tile(atlas::t::Chest), xp, yp, 8, 16, 1, 1);
}

void Stash::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 1, 0.6f);
}

Stash *Stash::near(Level &level) {
  if (!level.game.campaign)
    return nullptr;
  Stash *b = level.get<Stash>(level.game.campaign->chest);
  Player &p = *level.player;
  if (!b || p.dead())
    return nullptr;
  double xd = b->x - p.x, yd = b->y - p.y;
  return xd * xd + yd * yd < REACH * REACH ? b : nullptr;
}
