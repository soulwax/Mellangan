#include "entity/enemy/SupplyBarrel.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Magnet.hpp"
#include "entity/pickup/Nova.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
namespace t = atlas::t;

const EnemyType BARREL{t::Barrel, 12,  0, 0, 6, 1, 0, 0, {0.6f, 0.4f, 0.25f, 1},
                       false,     1e9f};
} // namespace

SupplyBarrel::SupplyBarrel() : Enemy(BARREL, 1, 0) {
  status.immune = (1u << int(StatusKind::Count)) - 1;
}

void SupplyBarrel::tick() {
  flash = std::max(0.0f, flash - DT);
  animT += DT;
  double xd = level->player->x - x, yd = level->player->y - y;
  if (xd * xd + yd * yd > 700 * 700)
    remove();
}

void SupplyBarrel::render(Screen &s) {
  s.flash(atlas::tile(t::Barrel), xp, yp, 8, 16, 1, 1, 0,
          flash > 0 ? 1.0f : 0.0f, col::white);
}

void SupplyBarrel::renderLight(Screen &s) {
  s.glow(xp, yp - 7 * scale, 50,
         Color{0.22f, 0.16f, 0.06f, 1} * (0.8f + 0.2f * std::sin(animT * 3)));
}

void SupplyBarrel::die() {
  remove();
  Sound::barrel.play(0.6f);
  Spark::burst(*level, x, bodyY(), type.blood, 12, 80, false);
  float r = rng.unit();
  if (r < 0.45f)
    level->add<Potion>(x, y);
  else if (r < 0.8f)
    level->add<Magnet>(x, y);
  else
    level->add<Nova>(x, y);
}
