#include "hero/ExpansionHero.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Bolt.hpp"
#include "level/Level.hpp"
#include "particle/debris/Chip.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Mark.hpp"
#include "particle/Flourish.hpp"
#include "particle/Drift.hpp"
#include "particle/Arc.hpp"
#include "particle/Spark.hpp"

namespace {
bool warden(const Codex &c) { return c.count(c.biomes) >= 3; }
bool tinkerer(const Codex &c) { return c.kills() >= 200; }
bool oracle(const Codex &c) { return c.shrines >= 2; }
bool corsair(const Codex &c) { return c.bestStreak >= 30; }

const HeroData DATA[] = {
    {"WARDEN", "Thorn Lash", "Keeper of root and stone",
     "Poison cannot take root", "walk 3 regions", warden, 84,
     {0.5f, 0.85f, 0.45f, 1}, 20, 0.96f, 0, 0, 0, 1, 0.25f, 0,
     "ROOTBOUND", "Heal 8; thorns damage and slow nearby foes for 2s", 10,
     {"Thorn Lash", "Heartroot", "Troll Blood"}},
    {"TINKERER", "Mortar", "Maker of midnight machines",
     "Every 15th kill repairs 8 health", "slay 200 foes", tinkerer, 84,
     {0.9f, 0.65f, 0.35f, 1}, 0, 1, 0, -0.05f, 8, 0, 0, 0,
     "SENTRY", "Deploy a six-second sentry that fires at nearby foes", 15,
     {"Mortar", "Ricochet Sling", "Hourglass Tonic"}},
    {"ORACLE", "Ground Sigil", "Reader of unwritten stars",
     "Gain 15% more experience", "kindle 2 shrines", oracle, 84,
     {0.7f, 0.6f, 1, 1}, -5, 1, 0.05f, 0, 0, 0, 0, 0.04f,
     "REVELATION", "Mark foes within 160: direct hits deal 35% more for 5s", 11,
     {"Ground Sigil", "Prism Beam", "Lucky Key"}},
    {"CORSAIR", "Harpoon", "Rover of the moonless coast",
     "Critical hits deal 220% damage", "reach a 30-kill streak", corsair, 84,
     {0.4f, 0.85f, 0.9f, 1}, -10, 1.12f, 0, 0, 7, 0, 0, 0.06f,
     "BROADSIDE", "Dash forward and fire five piercing shots", 8,
     {"Harpoon", "Boomerang", "Quicksilver"}},
};

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
    level->add<Bolt>(x, y - 7, xd / d * 190, yd / d * 190, damage, 1,
                     e.id, BoltStyle::Tinker)->hostile = hostile;
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
}

ExpansionHero::ExpansionHero(int variant)
    : Hero(DATA[std::clamp(variant, 0, 3)]),
      variant(std::clamp(variant, 0, 3)) {}

Sprite ExpansionHero::sprite() const { return atlas::expansion(10 + variant); }

void ExpansionHero::equip(Mob &p) const {
  Hero::equip(p);
  if (variant == 0)
    p.status.immune |= 1u << int(StatusKind::Poison);
}

void ExpansionHero::cast(Mob &p) const {
  if (p.dead())
    return;
  if (variant == 0) {
    p.mend(8);
    p.level->foes(p, p.x, p.y, 110, [&](Mob &e) {
      double xd = e.x - p.x, yd = e.y - p.y;
      float reach = 85 + e.radius;
      if (xd * xd + yd * yd > reach * reach)
        return;
      e.afflict(StatusKind::Chill, 2, 0.65f);
      e.strike(28 * p.might, xd, yd, 75);
      double d = std::max(1.0, std::sqrt(xd * xd + yd * yd));
      p.level->add<Arc>(std::vector<ArcNode>{{p.x + xd / d * 12, p.y + yd / d * 12},
                                             {e.x, e.y}},
                        0.3f, Color{0.35f, 0.75f, 0.25f, 1});
    });
    flourish::sigil(*p.level, p.x, p.y, 85, Color{0.45f, 0.9f, 0.4f, 1}, 0);
    flourish::scatter(*p.level, p.x, p.bodyY(), atlas::pt::Leaf, col::white, 12,
                      90, 30);
    Sound::nova.play(0.4f);
  } else if (variant == 1) {
    p.level->add<Sentry>(p.x, p.y, 16 * p.might, p.hostile());
    p.level->add<Shockwave>(p.x, p.y, 4, 30, 0.3f, col::gold, false);
    for (int i = 0; i < 6 && Debris::room(); i++)
      p.level->add<Chip>(p.x, p.y, 4, atlas::pt::Pebble,
                         Color{1, 0.85f, 0.55f, 1}, 1);
    Spark::burst(*p.level, p.x, p.y, col::gold, 8, 75, true);
    Sound::cast.play(0.4f);
  } else if (variant == 2) {
    p.level->foes(p, p.x, p.y, 184, [&](Mob &e) {
      double xd = e.x - p.x, yd = e.y - p.y;
      if (xd * xd + yd * yd <= 160 * 160) {
        e.afflict(StatusKind::Marked, 5, 0.35f);
        e.afflict(StatusKind::Chill, 1, 0.2f);
        if (rng.chance(0.5f))
          p.level->add<Mark>(e.x, e.bodyY() - 12, 1.2f, atlas::pt::ArcaneStar,
                             Color{0.85f, 0.7f, 1, 1}, 1, 0);
      }
    });
    p.level->add<Shockwave>(p.x, p.bodyY(), 10, 160, 0.6f,
                            Color{0.8f, 0.6f, 1, 0.8f});
    flourish::sigil(*p.level, p.x, p.bodyY(), 50, Color{0.8f, 0.6f, 1, 1}, 8);
    Sound::cast.play(0.5f);
  } else {
    double dir = std::atan2(p.yAim, p.xAim);
    for (int i = -2; i <= 2; i++) {
      double a = dir + i * 0.16;
      p.level->add<Arrow>(p.x, p.bodyY(), std::cos(a) * 270,
                          std::sin(a) * 270, 24 * p.might, 3,
                          ArrowStyle::Piercing)->hostile = p.hostile();
    }
    double x0 = p.x, y0 = p.bodyY();
    flourish::echo(*p.level, p.x, p.y, sprite(), Color{0.4f, 0.85f, 1, 1});
    for (int i = 0; i < 8; i++) {
      p.x += std::cos(dir) * 4;
      p.y += std::sin(dir) * 4;
      p.level->collide(p.x, p.y, p.radius);
    }
    p.shield(0.6f);
    flourish::streak(*p.level, x0, y0, p.x, p.bodyY(), atlas::pt::Puff,
                     Color{0.85f, 0.95f, 1, 0.6f}, 5);
    for (int i = 0; i < 6 && Drift::room(); i++) {
      Drift *d = p.level->add<Drift>(x0, y0, -std::cos(dir) * 60 + rng.range(-30, 30),
                                     -std::sin(dir) * 60 - rng.range(20, 50),
                                     atlas::pt::FrostGlint,
                                     Color{0.4f, 0.85f, 1, 1}, 0.45f);
      d->gravity = 200, d->drag = 0.5f, d->scale = 0.6f;
    }
    Sound::axe.play(0.5f);
  }
}

void ExpansionHero::slay(Mob &p, Enemy &) const {
  if (variant == 1 && ++p.heroCount % 15 == 0)
    p.mend(8);
}
