#include "item/weapon/Flurry.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Slash.hpp"
#include "level/Level.hpp"

Flurry::Flurry()
    : Upgrade("Flurry", atlas::tile(atlas::t::Dagger),
              {"Three quick cuts, side to side", "+5 damage", "Four cuts",
               "Faster flurries", "+5 damage, five cuts"},
              true) {
  owner = "DUELIST";
}

void Flurry::tick(Mob &p) {
  if (pending > 0 && (gap -= DT) <= 0) {
    pending--, gap = 0.12f, side = -side;
    p.level->add<Slash>(p, side, dmg, 0.9f, SlashStyle::Flurry)->source = this;
    Sound::slash.play(0.2f);
  }
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 1.1f : 1.5f) * p.haste * pace;
  pending = 3 + at(level, 3) + at(level, 5), gap = 0;
  dmg = (12.0f + 5 * (at(level, 2) + at(level, 5))) * p.might * power;
}
