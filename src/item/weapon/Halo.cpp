#include "item/weapon/Halo.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/effect/HaloField.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 5.0f + 2 * (Upgrade::at(l, 2) + Upgrade::at(l, 3) + Upgrade::at(l, 5)); }
float everyAt(int l) { return l >= 4 ? 0.38f : 0.5f; }
} // namespace

Halo::Halo()
    : Upgrade("Warding Halo", atlas::tile(atlas::t::PotionGrey),
              {"Scorches foes that get close", "Wider and hotter",
               "Wider and hotter", "Wider, pulses faster", "Wider and hotter"},
              true) {}

void Halo::levelUp(Mob &p) {
  Upgrade::levelUp(p);
  if (level == 1)
    p.level->add<HaloField>(*this, p);
}

void Halo::tick(Mob &p) {
  pulse *= std::exp(-6 * DT);
  if ((timer -= DT) > 0)
    return;
  timer = everyAt(level) * p.haste * pace;
  pulse = 1;
  float r = range(),
        dmg = damageAt(level) * p.might * power;
  bool scorched = false;
  p.level->foes(p, p.x, p.y, r + 24, [&](Mob &e) {
    double xd = e.x - p.x, yd = e.bodyY() - p.bodyY();
    float rr = r + e.radius;
    if (xd * xd + yd * yd < rr * rr)
      e.strike(dmg, e.x - p.x, e.y - p.y, 25, this), scorched = true;
  });
  hum ^= scorched;
  if (scorched && hum)
    Sound::orb.play(0.1f, 0.08f, 0.75f);
}

Figures Halo::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("radius", range(l));
}
