#include "item/weapon/Aegis.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Ward.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

namespace {
float everyAt(int l) { return 9.0f - 1.5f * (Upgrade::at(l, 3) + Upgrade::at(l, 5)); }
float wardAt(int l) { return 1.0f + 0.5f * (Upgrade::at(l, 2) + Upgrade::at(l, 4)); }
} // namespace

Aegis::Aegis()
    : Upgrade("Aegis", atlas::tile(atlas::t::Shield),
              {"A ward makes you untouchable for a moment", "Longer ward",
               "Wards more often", "Longer ward", "Wards more often"},
              true) {
  owner = "PRIESTESS";
}

void Aegis::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  float ward = wardAt(level);
  p.invuln = std::max(p.invuln, ward);
  p.level->add<Ward>(p, ward);
  Spark::burst(*p.level, p.x, p.bodyY(), {1, 0.92f, 0.6f, 1}, 8, 60, true);
  Sound::cast.play(0.3f);
}

Figures Aegis::figures(int l, bool) const {
  return Figures().add("ward", wardAt(l), Figure::Lasts).every(everyAt(l));
}
