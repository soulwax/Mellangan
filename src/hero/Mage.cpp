#include "hero/Mage.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
bool always(const Codex &) { return true; }
} // namespace

Mage::Mage()
    : Hero({"MAGE", "Arcane Staff", "Keeper of the old arcana",
            "+15% experience from every gem", "", always, 84,
            {0.78f, 0.55f, 1, 1}, 0, 1, 0, -0.05f, 0, 0, 0, 0, "BLINK",
            "Vanish and reappear ahead in an arcane burst", 6,
            {"Arcane Staff", "Hourglass Tonic", "Warding Halo"}}) {}

namespace {
void burst(Player &p) {
  p.level->enemiesNear(p.x, p.y, 80, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < 60 * 60)
      e.hurt(35 * p.might, xd, yd, 140);
  });
  Spark::burst(*p.level, p.x, p.bodyY(), {0.78f, 0.55f, 1, 1}, 18, 140, true);
}
} // namespace

void Mage::cast(Player &p) const {
  burst(p);
  p.x += p.xAim * 90, p.y += p.yAim * 90;
  p.level->collide(p.x, p.y, p.radius);
  p.invuln = std::max(p.invuln, 0.4f);
  burst(p);
  Sound::cast.play(0.6f, 0);
}
