#include "item/talent/EchoingBlink.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

EchoingBlink::EchoingBlink()
    : Upgrade("Echoing Blink", atlas::kit(atlas::kt::Lavender),
              {"Blink detonates where you land, recharges faster", "Blink detonates where you land, recharges faster", "Blink detonates where you land, recharges faster", "Blink detonates where you land, recharges faster", "Blink detonates where you land, recharges faster"},
              false) {
  owner = "MAGE";
}

void EchoingBlink::cast(Mob &p) {
  p.level->foes(p, p.x, p.y, 90, [&](Mob &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd < 70 * 70)
      e.strike(20.0f * level * p.might, xd, yd, 120, this);
  });
  p.skillTime *= 1 - 0.08f * level;
}
