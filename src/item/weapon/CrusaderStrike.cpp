#include "item/weapon/CrusaderStrike.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Slash.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 34.0f + 12 * (Upgrade::at(l, 2) + Upgrade::at(l, 5)); }
float everyAt(int l) { return l >= 4 ? 1.8f : 2.4f; }
float arcAt(int l) { return 1.5f + 0.25f * (Upgrade::at(l, 3) + Upgrade::at(l, 5)); }
} // namespace

CrusaderStrike::CrusaderStrike()
    : Upgrade("Crusader Strike", atlas::tile(atlas::t::Sword),
              {"A crushing cleave to both sides", "+12 damage", "Wider arc",
               "Faster strikes", "+12 damage, wider arc"},
              true) {
  owner = "KNIGHT";
}

void CrusaderStrike::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  float dmg = damageAt(level) * p.might * power,
        scale = arcAt(level);
  p.level->add<Slash>(p, 1, dmg, scale, SlashStyle::Crusader)->source = this;
  p.level->add<Slash>(p, -1, dmg, scale, SlashStyle::Crusader)->source = this;
  Sound::slash.play(0.4f);
}

Figures CrusaderStrike::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("arc", arcAt(l) * 100, Figure::Percent);
}
