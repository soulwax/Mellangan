#include "hero/Tinkerer.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bolt.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Chip.hpp"

namespace {

bool slayer(const Codex &c) { return c.kills() >= 200; }

class Sentry : public Entity {
public:
  float life = 6, timer = 0, damage;
  bool hostile;

  Sentry(double x, double y, float damage, bool hostile)
      : damage(damage), hostile(hostile) {
    this->x = x;
    this->y = y;
  }

  void tick() override {
    if ((life -= DT) <= 0 || level->player->dead())
      return remove();
    if ((timer -= DT) > 0)
      return;
    timer = 0.45f;
    auto targets = level->nearestFoes(hostile, x, y, 220, 1);
    if (targets.empty())
      return;
    Mob &e = *targets.front();
    double xd = e.x - x, yd = e.bodyY() - (y - 7);
    double d = std::sqrt(xd * xd + yd * yd);
    if (d < 1e-5)
      return;
    level->add<Bolt>(x, y - 7, xd / d * 190, yd / d * 190, damage, 1, e.id,
                     BoltStyle::Tinker)->hostile = hostile;
    Spark::burst(*level, x, y - 7, col::gold, 3, 30, true);
  }

  void render(Screen &s) override {
    s.sprite(atlas::frontier(atlas::fr::LanternPost), xp, yp, 8, 16);
    s.color(col::gold.alpha(0.5f * std::min(1.0f, life)));
    s.ring(xp, yp, 14);
    s.color(col::white);
  }

  void renderLight(Screen &s) override {
    s.glow(xp, yp - 7, 38, {0.15f, 0.1f, 0.03f, 1});
  }
};

} // namespace

Tinkerer::Tinkerer()
    : ExpansionHero({"TINKERER", "Mortar", "Maker of midnight machines",
                     "Every 15th kill repairs 8 health", "slay 200 foes", slayer, 84,
                     {0.9f, 0.65f, 0.35f, 1}, 0, 1, 0, -0.05f, 8, 0, 0, 0,
                     "SENTRY", "Deploy a six-second sentry that fires at nearby foes", 15,
                     {"Mortar", "Ricochet Sling", "Hourglass Tonic"}},
                    1) {}

void Tinkerer::cast(Mob &p) const {
  if (p.dead())
    return;
  p.level->add<Sentry>(p.x, p.y, 16 * p.might, p.hostile());
  p.level->add<Shockwave>(p.x, p.y, 4, 30, 0.3f, col::gold, false);
  for (int i = 0; i < 6 && Debris::room(); i++)
    p.level->add<Chip>(p.x, p.y, 4, atlas::pt::Pebble, Color{1, 0.85f, 0.55f, 1}, 1);
  Spark::burst(*p.level, p.x, p.y, col::gold, 8, 75, true);
  Sound::cast.play(0.4f);
}

void Tinkerer::slay(Mob &p, Enemy &) const {
  if (++p.heroCount % 15 == 0)
    p.mend(8);
}
