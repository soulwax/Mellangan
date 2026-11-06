#include "item/weapon/Flurry.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Slash.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 12.0f + 5 * (Upgrade::at(l, 2) + Upgrade::at(l, 5)); }
float everyAt(int l) { return l >= 4 ? 1.1f : 1.5f; }
int cutsAt(int l) { return 3 + Upgrade::at(l, 3) + Upgrade::at(l, 5); }
} // namespace

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
  cd = everyAt(level) * p.haste * pace;
  pending = cutsAt(level), gap = 0;
  dmg = damageAt(level) * p.might * power;
}

Figures Flurry::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("cuts", float(cutsAt(l)));
}
