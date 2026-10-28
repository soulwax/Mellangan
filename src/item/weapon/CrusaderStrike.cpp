#include "item/weapon/CrusaderStrike.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Slash.hpp"
#include "level/Level.hpp"

CrusaderStrike::CrusaderStrike()
    : Upgrade("Crusader Strike", atlas::tile(atlas::t::Sword),
              {"A crushing cleave to both sides", "+12 damage", "Wider arc",
               "Faster strikes", "+12 damage, wider arc"},
              true) {
  owner = "KNIGHT";
}

void CrusaderStrike::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 1.8f : 2.4f) * p.haste * pace;
  float dmg = (34.0f + 12 * (at(level, 2) + at(level, 5))) * p.might * power,
        scale = 1.5f + 0.25f * (at(level, 3) + at(level, 5));
  p.level->add<Slash>(p, 1, dmg, scale)->source = this;
  p.level->add<Slash>(p, -1, dmg, scale)->source = this;
  Sound::slash.play(0.4f);
}
