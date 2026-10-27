#include "item/weapon/Blade.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Slash.hpp"
#include "level/Level.hpp"

Blade::Blade()
    : Upgrade("Moon Blade", atlas::tile(atlas::t::Sword),
              {"Cleaves the side you face", "Also cleaves behind you",
               "+6 damage, wider arc", "+6 damage, faster swings",
               "+6 damage, wider arc"},
              true) {}

void Blade::tick(Player &p) {
  if ((cd -= DT) > 0)
    return;
  cd = (level >= 4 ? 1.05f : 1.3f) * p.haste * pace;
  float dmg =
      (16.0f + 6 * (at(level, 3) + at(level, 4) + at(level, 5))) * p.might * power;
  float scale = 1 + 0.2f * (at(level, 3) + at(level, 5));
  p.level->add<Slash>(p, p.facing, dmg, scale)->source = this;
  if (level >= 2)
    p.level->add<Slash>(p, -p.facing, dmg, scale)->source = this;
  Sound::slash.play(0.5f);
}
