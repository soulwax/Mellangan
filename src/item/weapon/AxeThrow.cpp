#include "item/weapon/AxeThrow.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Axe.hpp"
#include "level/Level.hpp"

AxeThrow::AxeThrow()
    : Upgrade("Hurled Axe", atlas::tile(atlas::t::Axe), {"Lobs a heavy spinning axe", "+10 damage", "Throws one more axe",
                                                        "+10 damage, more pierce", "Throws one more axe"}, true) {}

void AxeThrow::tick(Player &p) {
    if ((cd -= DT) > 0) return;
    cd = 2.3f * p.haste;
    Sound::axe.play(0.45f);
    int n = 1 + at(level, 3) + at(level, 5);
    float dmg = (26.0f + 10 * (at(level, 2) + at(level, 4))) * p.might;
    for (int i = 0; i < n; i++) {
        double xa = p.facing * rng.range(25, 70) + (i - (n - 1) * 0.5f) * 55, ya = -rng.range(230, 260);
        p.level->add<Axe>(p.x, p.bodyY(), xa, ya, dmg, 5 + 4 * at(level, 4), p.facing * 14);
    }
}
