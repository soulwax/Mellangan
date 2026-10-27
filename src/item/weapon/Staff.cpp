#include "item/weapon/Staff.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/projectile/Bolt.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

Staff::Staff()
    : Upgrade("Arcane Staff", atlas::tile(atlas::t::Staff), {"Fires bolts at the nearest foe", "+4 damage, casts faster",
                                                             "Fires one more bolt", "Bolts pierce one more foe",
                                                             "Fires one more bolt"}, true) {}

void Staff::tick(Player &p) {
    if ((cd -= DT) > 0) return;
    int n = 1 + at(level, 3) + at(level, 5);
    auto targets = p.level->nearest(p.x, p.y, 250, n);
    if (targets.empty()) return void(cd = 0);
    cd = (level >= 2 ? 0.85f : 1.05f) * p.haste;
    for (int i = 0; i < n; i++) {
        Enemy *e = targets[i % targets.size()];
        float spread = i < int(targets.size()) ? 0 : rng.range(-0.35f, 0.35f);
        double dir = std::atan2(e->bodyY() - p.bodyY(), e->x - p.x) + spread;
        p.level->add<Bolt>(p.x, p.bodyY(), std::cos(dir) * 230, std::sin(dir) * 230, (level >= 2 ? 14.0f : 10.0f) * p.might,
                           1 + at(level, 4), e->id);
    }
    Spark::burst(*p.level, p.x, p.bodyY(), {0.45f, 0.8f, 1, 1}, 4, 40, true);
    Sound::cast.play(0.3f, 0.1f);
}
