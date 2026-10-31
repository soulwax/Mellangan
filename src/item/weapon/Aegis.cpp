#include "item/weapon/Aegis.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/Ward.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

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
  cd = (9.0f - 1.5f * (at(level, 3) + at(level, 5))) * p.haste * pace;
  float ward = 1.0f + 0.5f * (at(level, 2) + at(level, 4));
  p.invuln = std::max(p.invuln, ward);
  p.level->add<Ward>(p, ward);
  Spark::burst(*p.level, p.x, p.bodyY(), {1, 0.92f, 0.6f, 1}, 8, 60, true);
  Sound::cast.play(0.3f);
}
