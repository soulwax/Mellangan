#include "entity/effect/HaloField.hpp"

#include "entity/Player.hpp"
#include "item/weapon/Halo.hpp"
#include "level/Level.hpp"

void HaloField::tick() { x = player.x, y = player.y; }

void HaloField::render(Screen &s) {
    s.color({0.7f, 0.55f, 1, 0.22f + 0.35f * halo.pulse});
    s.center(atlas::Ring, xp, yp - 4, halo.range() * 2 / atlas::Ring.w * (1 + 0.05f * halo.pulse), level->time * 0.6f);
    s.color(col::white);
}

void HaloField::renderLight(Screen &s) {
    s.glow(xp, yp, halo.range() * 2.8f, Color{0.10f, 0.05f, 0.20f, 1} * (1 + halo.pulse * 0.6f));
}
