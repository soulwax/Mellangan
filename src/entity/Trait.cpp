#include "entity/Trait.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/Hazard.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

const TraitDef TRAITS[int(Trait::Count)] = {
    {"hasted", {1, 0.95f, 0.5f, 1}},     {"vampiric", {0.95f, 0.2f, 0.25f, 1}},
    {"warded", {0.4f, 0.65f, 1, 1}},     {"splitting", {0.5f, 0.95f, 0.4f, 1}},
    {"volatile", {1, 0.5f, 0.12f, 1}},   {"frosted", {0.7f, 0.9f, 1, 1}},
    {"thorned", {0.7f, 0.8f, 0.3f, 1}},  {"regenerating", {0.4f, 1, 0.55f, 1}},
    {"blinking", {0.75f, 0.5f, 1, 1}},   {"phasing", {0.88f, 0.88f, 1, 1}},
    {"brooding", {0.85f, 0.55f, 0.35f, 1}}, {"gravitic", {0.55f, 0.4f, 0.95f, 1}},
    {"cursed", {0.6f, 0.55f, 0.5f, 1}},  {"silencing", {0.55f, 0.75f, 0.85f, 1}},
    {"fire enchanted", {1, 0.45f, 0.15f, 1}}, {"cold enchanted", {0.55f, 0.85f, 1, 1}},
    {"storm enchanted", {0.6f, 0.75f, 1, 1}}, {"stone skin", {0.65f, 0.6f, 0.55f, 1}},
    {"multishot", {1, 0.8f, 0.55f, 1}},  {"aura bearer", {1, 0.85f, 0.35f, 1}}};

namespace {

void nova(Mob &self, StatusKind kind, float power, Color c) {
  float damage = 6 + self.maxHp * 0.04f;
  self.level->foes(self, self.x, self.y, 70, [&](Mob &foe) {
    double xd = foe.x - self.x, yd = foe.y - self.y;
    if (xd * xd + yd * yd > 56 * 56)
      return;
    foe.strike(damage, xd, yd, 60);
    foe.afflict(kind, 2, power);
  });
  self.level->add<Shockwave>(self.x, self.bodyY(), 4, 56, 0.4f, c);
  Spark::burst(*self.level, self.x, self.bodyY(), c, 10, 80, true);
}

} // namespace

void Traits::add(Trait t) {
  bits |= 1u << int(t);
  if (t == Trait::Warded)
    ward = 5;
}

Color Traits::tone() const {
  for (int t = 0; t < int(Trait::Count); t++)
    if (has(Trait(t)))
      return TRAITS[t].color;
  return col::white;
}

float Traits::incoming(Mob &self, float damage) {
  calm = 2.5f;
  if (veiled())
    damage *= 0.1f;
  if (has(Trait::Thorned) && thorn <= 0) {
    auto near = self.level->nearestFoes(self, self.x, self.y, 64, 1);
    if (!near.empty()) {
      thorn = 0.4f;
      Mob &foe = *near[0];
      foe.strike(std::min(damage * 0.12f, 6 + self.maxHp * 0.01f), foe.x - self.x,
                 foe.y - self.y, 40);
      Spark::burst(*self.level, foe.x, foe.bodyY(), TRAITS[int(Trait::Thorned)].color, 4,
                   40, false);
    }
  }
  if (has(Trait::StoneSkin))
    damage *= 0.65f;
  if (has(Trait::Blinking) && blink <= 0 && damage > 0 && self.hp > damage) {
    blink = 3;
    flourish::echo(*self.level, self.x, self.y, self.body(),
                   TRAITS[int(Trait::Blinking)].color);
    float a = rng.range(0, TAU);
    self.x += std::cos(a) * 56, self.y += std::sin(a) * 56;
    self.level->collide(self.x, self.y, self.radius);
    Spark::burst(*self.level, self.x, self.bodyY(), TRAITS[int(Trait::Blinking)].color,
                 6, 50, true);
  }
  if (ward <= 0)
    return damage;
  if (--ward == 0) {
    self.level->add<Shockwave>(self.x, self.bodyY(), 4, 26 * self.scale, 0.3f,
                               TRAITS[int(Trait::Warded)].color);
    Sound::barrel.play(0.4f);
  }
  return damage * 0.25f;
}

void Traits::landed(Mob &self, float dealt) {
  if (dealt <= 0)
    return;
  if (self.hostile()) {
    Mob &target = *self.level->player;
    if (has(Trait::Cursed))
      target.afflict(StatusKind::Weaken, 3, 0.25f);
    if (has(Trait::Silencing))
      target.skillTime += 2;
    if (has(Trait::FireEnchanted))
      target.afflict(StatusKind::Burn, 2, 3);
    if (has(Trait::ColdEnchanted))
      target.afflict(StatusKind::Chill, 1.5f, 0.4f);
    if (has(Trait::StormEnchanted))
      target.afflict(StatusKind::Marked, 2, 0.2f);
  }
  if (!has(Trait::Vampiric))
    return;
  self.hp = std::min(self.maxHp, self.hp + std::max(dealt * 0.5f, self.maxHp * 0.02f));
  Spark::burst(*self.level, self.x, self.bodyY(), TRAITS[int(Trait::Vampiric)].color,
               4, 30, true);
}

void Traits::tick(Mob &self) {
  if (!bits)
    return;
  calm -= DT, blink -= DT, thorn -= DT, cycle += DT;
  if (has(Trait::Frosted) && (trail -= DT) <= 0)
    trail = 0.22f, self.level->add<FrostPatch>(self.x, self.y, self.hostile());
  if (has(Trait::Regenerating) && calm <= 0 && self.hp < self.maxHp) {
    self.hp = std::min(self.maxHp, self.hp + self.maxHp * 0.04f * DT);
    if (rng.chance(0.08f) && Spark::alive < 500)
      Spark::burst(*self.level, self.x, self.bodyY(),
                   TRAITS[int(Trait::Regenerating)].color, 1, 20, false);
  }
  if (has(Trait::Brooding) && (brood -= DT) <= 0) {
    brood = 8;
    self.split();
  }
  if (has(Trait::AuraBearer) && self.hostile() && (aura -= DT) <= 0) {
    aura = 0.5f;
    self.level->enemiesNear(self.x, self.y, 110, [&](Enemy &ally) {
      if (&ally != &self)
        ally.frenzy = std::max(ally.frenzy, 0.6f);
    });
  }
  if (has(Trait::Gravitic))
    self.level->foes(self, self.x, self.y, 130, [&](Mob &foe) {
      double xd = self.x - foe.x, yd = self.y - foe.y,
             d = std::sqrt(xd * xd + yd * yd);
      if (d < 130 && d > self.radius + foe.radius + 4) {
        foe.x += xd / d * 26 * DT, foe.y += yd / d * 26 * DT;
        self.level->collide(foe.x, foe.y, foe.radius);
      }
    });
}

void Traits::died(Mob &self, float damage) {
  if (has(Trait::Volatile))
    self.level->add<Blast>(self.x, self.y, damage, self.hostile());
  if (has(Trait::Splitting))
    self.split();
  if (has(Trait::FireEnchanted))
    nova(self, StatusKind::Burn, 4, TRAITS[int(Trait::FireEnchanted)].color);
  if (has(Trait::ColdEnchanted))
    nova(self, StatusKind::Chill, 0.5f, TRAITS[int(Trait::ColdEnchanted)].color);
  if (has(Trait::StormEnchanted))
    nova(self, StatusKind::Marked, 0.3f, TRAITS[int(Trait::StormEnchanted)].color);
}

void Traits::render(Screen &s, const Mob &self, float radius, float age) const {
  if (!any())
    return;
  Color c = tone();
  float beat = has(Trait::Volatile) ? 0.45f + 0.35f * std::sin(age * 14)
                                    : 0.5f + 0.15f * std::sin(age * 4);
  s.ring(self.xp, self.yp, radius + 3, c.alpha(beat));
  if (has(Trait::Gravitic))
    s.ring(self.xp, self.yp, radius + 10 - 6 * std::fmod(age, 1.0f),
           TRAITS[int(Trait::Gravitic)].color.alpha(0.4f * std::fmod(age, 1.0f)));
  if (has(Trait::Thorned))
    for (int i = 0; i < 6; i++) {
      float a = TAU * i / 6 + age * 0.5f, r = radius + 3;
      s.line(self.xp + std::cos(a) * r, self.yp + std::sin(a) * r * 0.6f,
             self.xp + std::cos(a) * (r + 3), self.yp + std::sin(a) * (r + 3) * 0.6f, 1,
             TRAITS[int(Trait::Thorned)].color.alpha(0.8f));
    }
  if (ward > 0)
    s.ring(self.xp, self.yp, radius + 6, c.alpha(0.3f + 0.08f * ward));
}
