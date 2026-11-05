#include "hero/Oracle.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Flourish.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"

namespace {
bool kindler(const Codex &c) { return c.shrines >= 2; }
} // namespace

Oracle::Oracle()
    : ExpansionHero({"ORACLE", "Ground Sigil", "Reader of unwritten stars",
                     "Every 12th kill marks the foes around it", "kindle 2 shrines", kindler, 84,
                     {0.7f, 0.6f, 1, 1}, -5, 1, 0.05f, 0, 0, 0, 0, 0.04f,
                     "REVELATION", "Mark foes within 160: direct hits deal 35% more for 5s", 11,
                     {"Ground Sigil", "Prism Beam", "Lucky Key"}},
                    2) {}

void Oracle::cast(Mob &p) const {
  if (p.dead())
    return;
  p.level->foes(p, p.x, p.y, 184, [&](Mob &e) {
    double xd = e.x - p.x, yd = e.y - p.y;
    if (xd * xd + yd * yd > 160 * 160)
      return;
    e.afflict(StatusKind::Marked, 5, 0.35f);
    e.afflict(StatusKind::Chill, 1, 0.2f);
    if (rng.chance(0.5f))
      p.level->add<Mark>(e.x, e.bodyY() - 12, 1.2f, atlas::pt::ArcaneStar,
                         Color{0.85f, 0.7f, 1, 1}, 1, 0);
  });
  p.level->add<Shockwave>(p.x, p.bodyY(), 10, 160, 0.6f, Color{0.8f, 0.6f, 1, 0.8f});
  flourish::sigil(*p.level, p.x, p.bodyY(), 50, Color{0.8f, 0.6f, 1, 1}, 8);
  Sound::cast.play(0.5f);
}

void Oracle::slay(Mob &p, Enemy &e) const {
  if (++p.heroCount % 12)
    return;
  p.level->foes(p, e.x, e.y, 72, [&](Mob &f) {
    double xd = f.x - e.x, yd = f.y - e.y;
    if (xd * xd + yd * yd <= 64 * 64)
      f.afflict(StatusKind::Marked, 4, 0.25f);
  });
  p.level->add<Shockwave>(e.x, e.bodyY(), 6, 64, 0.4f, Color{0.8f, 0.6f, 1, 0.5f});
}
