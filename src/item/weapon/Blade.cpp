#include "item/weapon/Blade.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Slash.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 16.0f + 6 * (Upgrade::at(l, 3) + Upgrade::at(l, 4) + Upgrade::at(l, 5)); }
float everyAt(int l) { return l >= 4 ? 1.05f : 1.3f; }
float arcAt(int l) { return 1 + 0.2f * (Upgrade::at(l, 3) + Upgrade::at(l, 5)); }
} // namespace

Blade::Blade()
    : Upgrade("Moon Blade", atlas::tile(atlas::t::Sword),
              {"Cleaves the side you face", "Also cleaves behind you",
               "+6 damage, wider arc", "+6 damage, faster swings",
               "+6 damage, wider arc"},
              true) {}

void Blade::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  cd = everyAt(level) * p.haste * pace;
  float dmg =
      damageAt(level) * p.might * power;
  float scale = arcAt(level);
  p.level->add<Slash>(p, p.facing, dmg, scale)->source = this;
  if (level >= 2)
    p.level->add<Slash>(p, -p.facing, dmg, scale)->source = this;
  Sound::slash.play(0.5f);
}

Figures Blade::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("sides", 1.0f + Upgrade::at(l, 2)).add("arc", arcAt(l) * 100, Figure::Percent);
}
