#include "item/weapon/FrostNova.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Chip.hpp"

namespace {
constexpr Color ICE{0.85f, 0.95f, 1, 0.95f}, CYAN{0.45f, 0.8f, 1, 0.6f};

void bloom(Level &level, double x, double y, float r) {
  level.add<Shockwave>(x, y, 8, r, 0.35f, ICE);
  level.add<Shockwave>(x, y, 4, r * 0.8f, 0.55f, CYAN, false);
  level.add<Mark>(x, y, 0.7f, atlas::Glow, Color{0.6f, 0.85f, 1, 0.11f},
                  r * 2 / atlas::Glow.w);
  for (int i = 0; i < 12 && Debris::room(); i++) {
    float dir = TAU * i / 12 + rng.range(-0.2f, 0.2f), v = r / 24;
    Chip *c = level.add<Chip>(x + std::cos(dir) * 6, y + std::sin(dir) * 6, 4,
                              atlas::pt::IceShard, col::white, 1);
    c->xa = std::cos(dir) * v, c->ya = std::sin(dir) * v, c->za = 0.7;
    c->rot = dir + PI * 0.25f, c->spin = 0;
  }
  for (int i = 0; i < 8; i++) {
    float dir = rng.range(0, TAU), d = r * std::sqrt(rng.range(0.15f, 1));
    level.add<Mark>(x + std::cos(dir) * d, y + std::sin(dir) * d,
                    rng.range(0.8f, 1.4f),
                    i % 3 ? atlas::pt::FrostGlint : atlas::pt::Snowflake,
                    ICE.alpha(0.8f));
  }
  Spark::burst(level, x, y, ICE, 8, r * 1.6f, true);
}
} // namespace

FrostNova::FrostNova()
    : Upgrade("Frost Nova", atlas::kit(atlas::kt::FrozenShrub),
              {"Frost blasts slow foes by 35% for 2s", "+8 damage", "Wider burst",
               "Faster bursts", "+8 damage, wider burst"},
              true) {
  owner = "MAGE";
}

void FrostNova::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 2.6f : 3.4f) * p.haste * pace;
  float r = 60.0f + 14 * (at(level, 3) + at(level, 5)),
        dmg = (18.0f + 8 * (at(level, 2) + at(level, 5))) * p.might * power;
  p.level->enemiesNear(p.x, p.y, r + 24, [&](Enemy &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd < rr * rr) {
      e.afflict(StatusKind::Chill, 2.0f, 0.35f, this);
      e.hurt(dmg, xd, yd, 180, this);
    }
  });
  bloom(*p.level, p.x, p.bodyY(), r);
  Sound::cast.play(0.35f);
}
