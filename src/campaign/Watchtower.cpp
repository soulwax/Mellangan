#include "campaign/Watchtower.hpp"

#include <cmath>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/projectile/Arrow.hpp"
#include "level/Level.hpp"

namespace {

namespace tt = atlas::tt;
constexpr Color FLAME{0.55f, 0.3f, 0.08f, 1};

} // namespace

void Watchtower::onAdded() {
  level->tag(this);
  level->collide(x, y, 6);
}

void Watchtower::tick() {
  t += DT;
  if ((reload -= DT) > 0)
    return;
  double top = y - 40;
  Enemy *best = nullptr;
  double bestD = RANGE * RANGE;
  level->enemiesNear(x, top, RANGE, [&](Enemy &e) {
    if (e.removed || e.buried || e.hp <= 0)
      return;
    double xd = e.x - x, yd = e.bodyY() - top, d2 = xd * xd + yd * yd;
    if (d2 < bestD)
      bestD = d2, best = &e;
  });
  if (!best)
    return;
  reload = RELOAD, shots++;
  double xd = best->x - x, yd = best->bodyY() - top, d = std::max(1.0, std::sqrt(bestD));
  float damage = 8 + 2.0f * level->player->lvl;
  level->add<Arrow>(x, top, xd / d * 260, yd / d * 260, damage, 1);
  Sound::bow.play(0.12f, 0.1f, 1.2f);
}

void Watchtower::render(Screen &s) {
  s.sprite(atlas::town(tt::Bricks), xp, yp, 8, 16, 1, 1);
  s.sprite(atlas::town(tt::ArrowSlit), xp, yp - 16, 8, 16, 1, 1);
  s.sprite(atlas::town(tt::TowerTop + 25), xp, yp - 32, 8, 16, 1, 1);
  s.sprite(atlas::town(tt::TowerTop + 1), xp, yp - 48, 8, 16, 1, 1);
}

void Watchtower::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 1.2f, 0.8f);
}

void Watchtower::renderLight(Screen &s) {
  s.glow(xp, yp - 40, 70, FLAME * (0.8f + 0.2f * std::sin(t * 6)));
  level->lights.add(x, y - 30, 70, 0.6f);
}
