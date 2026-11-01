#include "hero/Warden.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Arc.hpp"
#include "particle/Flourish.hpp"

namespace {
bool walker(const Codex &c) { return c.count(c.biomes) >= 3; }
} // namespace

Warden::Warden()
    : ExpansionHero({"WARDEN", "Thorn Lash", "Keeper of root and stone",
                     "Poison cannot take root", "walk 3 regions", walker, 84,
                     {0.5f, 0.85f, 0.45f, 1}, 20, 0.96f, 0, 0, 0, 1, 0.25f, 0,
                     "ROOTBOUND", "Heal 8; thorns damage and slow nearby foes for 2s", 10,
                     {"Thorn Lash", "Heartroot", "Troll Blood"}},
                    0) {}

void Warden::equip(Mob &p) const {
  Hero::equip(p);
  p.status.immune |= 1u << int(StatusKind::Poison);
}

void Warden::cast(Mob &p) const {
  if (p.dead())
    return;
  p.mend(8);
  p.level->foes(p, p.x, p.y, 110, [&](Mob &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    float reach = 85 + e.radius;
    if (xd * xd + yd * yd > reach * reach)
      return;
    e.afflict(StatusKind::Chill, 2, 0.65f);
    e.strike(28 * p.might, xd, yd, 75);
    double d = std::max(1.0, std::sqrt(xd * xd + yd * yd));
    p.level->add<Arc>(std::vector<ArcNode>{{p.x + xd / d * 12, p.y + yd / d * 12},
                                           {e.x, e.y}},
                      0.3f, Color{0.35f, 0.75f, 0.25f, 1});
  });
  flourish::sigil(*p.level, p.x, p.y, 85, Color{0.45f, 0.9f, 0.4f, 1}, 0);
  flourish::scatter(*p.level, p.x, p.bodyY(), atlas::pt::Leaf, col::white, 12, 90, 30);
  Sound::nova.play(0.4f);
}
