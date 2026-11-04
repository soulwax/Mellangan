#include "entity/Ally.hpp"
#include "entity/ability/Common.hpp"
#include "entity/effect/Boon.hpp"
#include "item/weapon/Recipe.hpp"
#include "particle/Mark.hpp"

namespace ability {

namespace {

constexpr Color SMOKE{0.6f, 0.6f, 0.65f, 1}, BLOOD{0.85f, 0.2f, 0.3f, 1},
                VOID{0.55f, 0.3f, 0.95f, 1}, GOLD{1, 0.85f, 0.45f, 1},
                FROST{0.6f, 0.85f, 1, 1}, SIGHT{0.75f, 0.65f, 1, 1}, WIND{0.85f, 0.95f, 1, 1},
                PRISM{0.8f, 0.6f, 1, 1}, STORM{0.55f, 0.75f, 1, 1}, HOLY{1, 0.95f, 0.75f, 1},
                STONE{0.75f, 0.65f, 0.5f, 1}, BRASS{1, 0.75f, 0.35f, 1},
                ALCHEMY{0.95f, 0.85f, 0.35f, 1};

Enemy *asFoe(Mob &m) { return m.hostile() ? static_cast<Enemy *>(&m) : nullptr; }

void boon(Mob &m, float Mob::*stat, float delta, float seconds, Color tone,
          float delay = 0) {
  m.level->add<Boon>(m, stat, delta, seconds, delay, tone);
}

void aimPoint(Mob &caster, double reach, double &x, double &y) {
  double ax, ay;
  caster.aim(ax, ay);
  x = caster.x + ax * reach, y = caster.y + ay * reach;
}

} // namespace

void smoke(Mob &caster, float radius, float seconds) {
  Level &level = *caster.level;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(6 * caster.power(), xd, yd, 60);
    foe.afflict(StatusKind::Fear, seconds, 1);
  });
  caster.shield(1.5f);
  for (int i = 0; i < 16 && Drift::room(); i++) {
    float a = rng.range(0, TAU), r = rng.range(4, radius * 0.6f);
    Drift *d = level.add<Drift>(caster.x + std::cos(a) * r, caster.y + std::sin(a) * r * 0.7f,
                                std::cos(a) * 12, -rng.range(4, 10), atlas::pt::Puff,
                                SMOKE.alpha(0.55f), rng.range(1.2f, 1.8f));
    d->grow = 2, d->scale = 0.9f;
  }
  Sound::barrel.play(0.4f, 0, 0.8f);
}

void rite(Mob &caster, float cost, float might, float seconds) {
  caster.hp = std::max(1.0f, caster.hp - caster.maxHp * cost);
  caster.flash = 1;
  boon(caster, &Mob::might, might, seconds, BLOOD);
  within(caster, 70, [&](Mob &foe, double xd, double yd) {
    foe.strike(10 * caster.power(), xd, yd, 80);
  });
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 36, BLOOD, 8);
  caster.level->add<Shockwave>(caster.x, caster.bodyY(), 4, 70, 0.4f, BLOOD.alpha(0.8f));
  Sound::scythe.play(0.5f, 0, 0.8f);
}

void well(Mob &caster, float reach, float radius, float damage) {
  Level &level = *caster.level;
  double wx, wy;
  aimPoint(caster, reach, wx, wy);
  level.foesOf(caster, wx, wy, radius + 20, [&](Mob &foe) {
    double xd = foe.x - wx, yd = foe.y - wy;
    if (xd * xd + yd * yd >= radius * radius)
      return;
    foe.strike(damage * caster.power(), xd, yd, -380);
    foe.afflict(StatusKind::Chill, 1.5f, 0.4f);
  });
  level.add<Shockwave>(wx, wy - 7, radius, 4, 0.5f, VOID.alpha(0.85f), false);
  flourish::sigil(level, wx, wy - 7, radius * 0.5f, VOID, 10);
  Sound::nova.play(0.5f, 0, 0.6f);
}

void mirror(Mob &caster, float seconds) {
  if (caster.hostile())
    return tumble(caster, 70);
  double x, y;
  aimPoint(caster, 50, x, y);
  summon(*caster.level, AllyKind::Decoy, x, y, 0, seconds);
  flourish::echo(*caster.level, caster.x, caster.y, caster.body(), PRISM);
  Sound::cast.play(0.4f, 0, 1.3f);
}

void horn(Mob &caster, float radius, float seconds) {
  Level &level = *caster.level;
  if (Enemy *self = asFoe(caster)) {
    self->frenzy = seconds;
    level.enemiesNear(caster.x, caster.y, radius, [&](Enemy &e) { e.frenzy = seconds; });
  } else {
    boon(caster, &Mob::speed, caster.speed * 0.15f, seconds, GOLD);
    for (Ally *a : level.allies)
      if (!a->removed)
        boon(*a, &Mob::haste, -0.3f, seconds, GOLD);
  }
  within(caster, 60, [&](Mob &foe, double xd, double yd) {
    foe.strike(6 * caster.power(), xd, yd, 120);
  });
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, radius, 0.5f, GOLD.alpha(0.7f));
  Sound::howl.play(0.5f, 0, 1.3f);
}

void shatter(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    if (foe.status.power(StatusKind::Chill) > 0) {
      foe.status.end(StatusKind::Chill);
      foe.strike(damage * 2 * caster.power(), xd, yd, 120);
      flourish::scatter(level, foe.x, foe.bodyY(), atlas::pt::IceShard, FROST, 4, 50, -30);
    } else {
      foe.strike(damage * 0.4f * caster.power(), xd, yd, 40);
      foe.afflict(StatusKind::Chill, 2, 0.35f);
    }
  });
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, radius, 0.45f, FROST.alpha(0.85f));
  Sound::nova.play(0.5f, 0, 1.4f);
}

void sight(Mob &caster, float radius) {
  Level &level = *caster.level;
  if (!caster.hostile())
    level.chart.unveil(caster.x, caster.y, 12);
  level.foesOf(caster, caster.x, caster.y, radius, [&](Mob &foe) {
    Enemy *e = asFoe(foe);
    bool marked = e && (e->elite || e->title || e->boss);
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (marked || xd * xd + yd * yd < 160 * 160) {
      foe.afflict(StatusKind::Marked, marked ? 8.0f : 3.0f, marked ? 0.3f : 0.15f);
      foe.strike(4 * caster.power(), xd, yd, 0);
    }
  });
  level.add<Shockwave>(caster.x, caster.bodyY(), 10, radius, 0.7f, SIGHT.alpha(0.6f), false);
  flourish::sigil(level, caster.x, caster.bodyY(), 30, SIGHT, 6);
  Sound::cast.play(0.4f, 0, 0.8f);
}

void sprint(Mob &caster, float seconds) {
  if (Enemy *self = asFoe(caster))
    self->frenzy = seconds;
  else
    boon(caster, &Mob::speed, caster.speed * 0.5f, seconds, WIND);
  caster.shield(0.4f);
  flourish::echo(*caster.level, caster.x, caster.y, caster.body(), WIND);
  Sound::slash.play(0.4f, 0, 1.5f);
}

void prison(Mob &caster, float radius, float seconds, float damage) {
  Level &level = *caster.level;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), xd, yd, 0);
    foe.afflict(StatusKind::Chill, seconds, 0.9f);
  });
  for (int i = 0; i < 16; i++) {
    float a = TAU * i / 16;
    level.add<Mark>(caster.x + std::cos(a) * radius, caster.y + std::sin(a) * radius * 0.8f,
                    seconds, atlas::pt::ArcaneStar, i % 2 ? PRISM : FROST, 1.2f, a);
  }
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, radius, 0.4f, PRISM.alpha(0.8f));
  Sound::nova.play(0.5f, 0, 1.2f);
}

void hawk(Mob &caster, float seconds, float damage) {
  if (caster.hostile())
    return hunt(caster, 2, damage * 0.5f);
  summon(*caster.level, AllyKind::Hawk, caster.x, caster.y - 10, damage * caster.power(),
         seconds, pay::Bleed);
  Sound::howl.play(0.4f, 0, 1.8f);
}

void stand(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  if (caster.hp < caster.maxHp * 0.3f) {
    caster.shield(4);
    level.add<Ward>(caster, 4);
    boon(caster, &Mob::regen, 25.0f / 4, 4, GOLD);
  }
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), xd, yd, 200);
  });
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, radius, 0.4f, GOLD);
  flourish::sigil(level, caster.x, caster.bodyY(), 50, GOLD, 12);
  Sound::raise.play(0.5f, 0, 0.9f);
}

void transmute(Mob &caster, float radius, float seconds) {
  Level &level = *caster.level;
  if (!caster.hostile())
    level.gilds.push_back({caster.x, caster.y, radius, level.time + seconds});
  std::erase_if(level.gilds, [&](const Gild &g) { return g.until < level.time; });
  level.add<Aura>(caster, AuraDef{radius, seconds, 0, 0, 0, 4 * caster.power(), 0, 0, 0, 0,
                                  false, ALCHEMY, atlas::pt::Coin});
  flourish::sigil(level, caster.x, caster.bodyY(), radius * 0.7f, ALCHEMY, 12);
  Sound::potion.play(0.5f, 0, 0.8f);
}

void storm(Mob &caster, float seconds, float damage) {
  caster.level->add<Aura>(caster, AuraDef{110, seconds, 0, 0, 0, 0, 0, 0, 0.33f,
                                          damage * caster.power(), true, STORM,
                                          atlas::pt::Wisp});
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 40, STORM, 8);
  Sound::thunder.play(0.4f);
}

void raise(Mob &caster, int count, float seconds, float damage) {
  if (caster.hostile())
    return drain(caster, 80, damage * 0.5f);
  for (int i = 0; i < count; i++) {
    float a = TAU * i / count;
    summon(*caster.level, AllyKind::Skeleton, caster.x + std::cos(a) * 22,
           caster.y + std::sin(a) * 16, damage * caster.power(), seconds);
  }
  Sound::raise.play(0.5f, 0);
}

void mercy(Mob &caster, int wards) {
  caster.traits.ward = std::max(caster.traits.ward, wards);
  caster.mend(10);
  within(caster, 60, [&](Mob &foe, double xd, double yd) {
    foe.strike(8 * caster.power(), xd, yd, 120);
  });
  caster.level->add<Ward>(caster, 2);
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 34, HOLY, 12);
  Sound::nova.play(0.4f, 0, 1.5f);
}

void parry(Mob &caster, float window) {
  caster.parry = window;
  within(caster, 50, [&](Mob &foe, double xd, double yd) {
    foe.strike(10 * caster.power(), xd, yd, 100);
  });
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 22, col::gold, 4);
  Sound::slash.play(0.5f, 0, 1.6f);
}

void rampart(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  double ax, ay, cx, cy;
  caster.aim(ax, ay);
  aimPoint(caster, reach, cx, cy);
  double tx = -ay, ty = ax;
  level.foesOf(caster, cx, cy, 90, [&](Mob &foe) {
    double ex = foe.x - cx, ey = foe.y - cy;
    double across = ex * ax + ey * ay, along = ex * tx + ey * ty;
    if (std::abs(along) > 60 || std::abs(across) > 18 + foe.radius)
      return;
    foe.strike(damage * caster.power(), ax, ay, 240);
    foe.afflict(StatusKind::Chill, 1, 0.65f);
  });
  for (int i = -5; i <= 5; i++)
    for (int k = 0; k < 2 && Debris::room(); k++)
      level.add<Chip>(cx + tx * i * 11, cy + ty * i * 11, 4,
                      k ? atlas::pt::Rock : atlas::pt::Pebble, col::white, 1.1);
  level.add<Shockwave>(cx, cy, 4, 60, 0.35f, STONE.alpha(0.8f), false);
  Sound::erupt.play(0.5f, 0);
}

void overclock(Mob &caster, float seconds, float jam) {
  if (Enemy *self = asFoe(caster))
    self->frenzy = seconds;
  else {
    boon(caster, &Mob::haste, -caster.haste * 0.5f, seconds, BRASS);
    boon(caster, &Mob::haste, 1, jam, SMOKE, seconds);
  }
  within(caster, 60, [&](Mob &foe, double xd, double yd) {
    foe.strike(8 * caster.power(), xd, yd, 80);
  });
  Spark::burst(*caster.level, caster.x, caster.bodyY(), BRASS, 14, 90, true);
  Sound::cast.play(0.5f, 0, 1.5f);
}

void foresight(Mob &caster, int count) {
  for (Mob *foe : caster.level->nearestFoes(caster, caster.x, caster.y, 200, count)) {
    foe->afflict(StatusKind::Marked, 4, 0.2f);
    foe->strike(6 * caster.power(), foe->x - caster.x, foe->y - caster.y, 0);
    flourish::scatter(*caster.level, foe->x, foe->bodyY(), atlas::pt::ArcaneStar, SIGHT, 3,
                      30, -30);
  }
  caster.sure = std::max(caster.sure, count);
  Sound::cast.play(0.4f, 0, 1.2f);
}

void cannonade(Mob &caster, int count, float damage) {
  double ax, ay;
  caster.aim(ax, ay);
  for (int i = 0; i < count; i++) {
    double d = 40 + 26 * i;
    lob(caster, caster.x + ax * d, caster.y + ay * d, damage, 0, 0, 0.1f);
  }
  Sound::explosion.play(0.3f, 0.1f);
}

} // namespace ability
