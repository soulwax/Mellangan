#include "entity/Trait.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "entity/enemy/Hazard.hpp"
#include "level/Level.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

const TraitDef TRAITS[int(Trait::Count)] = {
    {"hasted", {1, 0.95f, 0.5f, 1}},     {"vampiric", {0.95f, 0.2f, 0.25f, 1}},
    {"warded", {0.4f, 0.65f, 1, 1}},     {"splitting", {0.5f, 0.95f, 0.4f, 1}},
    {"volatile", {1, 0.5f, 0.12f, 1}},   {"frosted", {0.7f, 0.9f, 1, 1}}};

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
  if (!has(Trait::Vampiric) || dealt <= 0)
    return;
  self.hp = std::min(self.maxHp, self.hp + std::max(dealt * 0.5f, self.maxHp * 0.02f));
  Spark::burst(*self.level, self.x, self.bodyY(), TRAITS[int(Trait::Vampiric)].color,
               4, 30, true);
}

void Traits::tick(Mob &self) {
  if (has(Trait::Frosted) && (trail -= DT) <= 0)
    trail = 0.22f, self.level->add<FrostPatch>(self.x, self.y, self.hostile());
}

void Traits::died(Mob &self, float damage) {
  if (has(Trait::Volatile))
    self.level->add<Blast>(self.x, self.y, damage, self.hostile());
  if (has(Trait::Splitting))
    self.split();
}

void Traits::render(Screen &s, const Mob &self, float radius, float age) const {
  if (!any())
    return;
  Color c = tone();
  float beat = has(Trait::Volatile) ? 0.45f + 0.35f * std::sin(age * 14)
                                    : 0.5f + 0.15f * std::sin(age * 4);
  s.ring(self.xp, self.yp, radius + 3, c.alpha(beat));
  if (ward > 0)
    s.ring(self.xp, self.yp, radius + 6, c.alpha(0.3f + 0.08f * ward));
}
