#include "item/weapon/Transmute.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Gem.hpp"
#include "particle/Mark.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

namespace {
float damageAt(int l) { return 110.0f + 40 * Upgrade::at(l, 2); }
float everyAt(int l) { return l >= 4 ? 4.0f : 5.5f; }
float goldAt(int l) { return 2.0f + 2 * Upgrade::at(l, 3); }
} // namespace

Transmute::Transmute()
    : Upgrade("Transmute", atlas::tile(atlas::t::Coin),
              {"Turns a foe to gold: heavy damage, rich gems", "+40 damage",
               "Richer gold", "Faster transmutation", "Transmutes two foes"},
              true) {
  owner = "ALCHEMIST";
}

void Transmute::tick(Mob &p) {
  if ((cd -= DT) > 0)
    return;
  auto targets = p.level->nearestFoes(p, p.x, p.y, 200, 2);
  if (targets.empty())
    return void(cd = 0.3f);
  cd = everyAt(level) * p.haste * pace;
  for (int i = 0; i <= at(level, 5) && i < int(targets.size()); i++) {
    Mob *e = targets[i];
    double x = e->x, y = e->y;
    Enemy *gold = p.hostile() ? nullptr : static_cast<Enemy *>(e);
    if (gold)
      gold->gilded = true;
    e->strike(damageAt(level) * p.might * power, 0, 0, 0, this);
    if (gold)
      gold->gilded = gold->removed;
    if (e->removed)
      Gem::drop(*p.level, x, y, goldAt(level));
    p.level->add<Mark>(x, y - 8, 0.6f, atlas::pt::HolyGlint, col::gold, 1.5f);
    Spark::burst(*p.level, x, y - 6, col::gold, 8, 70, true);
  }
  Sound::chest.play(0.3f);
}

Figures Transmute::figures(int l, bool) const {
  return Figures().damage(damageAt(l)).every(everyAt(l)).add("foes", 1.0f + Upgrade::at(l, 5)).add("gold", goldAt(l));
}
