#include "item/weapon/Aegis.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

Aegis::Aegis()
    : Upgrade("Aegis", atlas::tile(atlas::t::Shield),
              {"A ward makes you untouchable for a moment", "Longer ward",
               "Wards more often", "Longer ward", "Wards more often"},
              true) {
  owner = "PRIESTESS";
}

void Aegis::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (9.0f - 1.5f * (at(level, 3) + at(level, 5))) * p.haste * pace;
  p.invuln = std::max(p.invuln, 1.0f + 0.5f * (at(level, 2) + at(level, 4)));
  Spark::burst(*p.level, p.x, p.bodyY(), {1, 0.92f, 0.6f, 1}, 18, 90, true);
  Sound::cast.play(0.3f);
}
