#include "campaign/Waypoint.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

namespace fr = atlas::fr;

constexpr Color RUNE{0.45f, 0.85f, 1, 1};

} // namespace

void Waypoint::onAdded() {
  level->tag(this);
  level->collide(x, y, 6);
}

void Waypoint::tick() {
  t += DT;
  Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  bool near = !p.dead() && xd * xd + yd * yd < REACH * REACH;
  if (near && !inside) {
    p.hp = p.maxHp;
    level->game.saveCampaign();
    level->add<Shockwave>(x, y - 10, 4, 34, 0.5f, RUNE, false);
    Spark::burst(*level, x, y - 12, RUNE, 10, 50, true);
    Sound::potion.play(0.5f, 0, 1.2f);
    level->game.announce("THE WAYPOINT KEEPS YOUR PATH");
  }
  inside = near;
}

void Waypoint::render(Screen &s) {
  s.sprite(atlas::frontier(fr::RuneStoneLow), xp, yp, 8, 16, 1, 1);
  s.sprite(atlas::frontier(fr::RuneStoneHigh), xp, yp - 16, 8, 16, 1, 1);
  float k = 0.6f + 0.4f * std::sin(t * 2.5f);
  s.rect(xp - 1, yp - 22, 2, 2, RUNE.alpha(k));
}

void Waypoint::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 1, 0.7f);
}

void Waypoint::renderLight(Screen &s) {
  s.glow(xp, yp - 14, 70, RUNE * (0.25f + 0.08f * std::sin(t * 2.5f)));
  level->lights.add(x, y, 64, 0.6f);
}

Waypoint *Waypoint::near(Level &level) {
  if (!level.game.campaign)
    return nullptr;
  Player &p = *level.player;
  for (entt::entity id : level.game.campaign->waypoints)
    if (Waypoint *w = level.get<Waypoint>(id)) {
      double xd = w->x - p.x, yd = w->y - p.y;
      if (!p.dead() && xd * xd + yd * yd < REACH * REACH)
        return w;
    }
  return nullptr;
}
