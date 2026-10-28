#include "item/relic/ExpansionRelics.hpp"

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Chest.hpp"
#include "item/weapon/ExpansionWeapons.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
const char *const NAMES[] = {"Carnage Bell", "Last Bastion", "Pilgrim Compass",
                            "Plague Seed", "Recall Mirror", "Weather Vane",
                            "Hunter's Writ", "Mercy Locket"};
const char *const TEXT[] = {
    "Every 10th streak kill blasts 16 foes; 3s cooldown",
    "Below 30% health halve one hit; 8s cooldown",
    "Find shrines within 12 regions; kindle 25% faster",
    "Kills poison 6 nearby foes for 3s; 0.6s cooldown",
    "Class skills call 2 returning blades; 8s cooldown",
    "Every 3s cleanse afflictions and pulse biome magic",
    "First 5 elite kills grant treasure; 20s cooldown",
    "Store 40% of a hit, heal after 3s; cap 20, 8s cooldown",
};

bool coldShrine(const Structure *s, const void *ctx) {
  return s->kind == StructureKind::Shrine &&
         !static_cast<const Night *>(ctx)->isLit(*s);
}
}

ExpansionRelic::ExpansionRelic(int kind)
    : Upgrade(NAMES[std::clamp(kind, 0, 7)],
              atlas::expansion(22 + std::clamp(kind, 0, 7)),
              {TEXT[std::clamp(kind, 0, 7)], "", "", "", ""}, false),
      kind(std::clamp(kind, 0, 7)) {
  rare = true;
}

void ExpansionRelic::pulse(Player &p, double x, double y, float damage,
                           int status) {
  int left = kind == 0 ? 16 : 6;
  p.level->enemiesNear(x, y, 115, [&](Enemy &e) {
    double dx = e.x - x, dy = e.bodyY() - y;
    if (left <= 0 || dx * dx + dy * dy > 100 * 100)
      return;
    left--;
    if (status >= 0)
      e.afflict(StatusKind(status), 3, status == int(StatusKind::Chill) ?
                                    0.35f : 5 * p.might, this);
    if (damage > 0)
      e.hurt(damage * p.might, dx, dy, 60, this);
  });
  Spark::burst(*p.level, x, y, kind == 3 ? col::heal : col::gold, 10, 100, true);
}

void ExpansionRelic::tick(Player &p) {
  cd = std::max(0.0f, cd - DT);
  if (kind == 2) {
    p.level->kindleRate *= 1.25f;
    if (cd > 0)
      return;
    cd = 2;
    Night &night = p.level->night;
    Structure shrine;
    if (world::nearestStructure(p.x, p.y, 12, shrine, coldShrine, &night))
      night.beacon = shrine, night.hasBeacon = true;
  } else if (kind == 5 && cd <= 0) {
    cd = 3;
    p.status.effects[int(StatusKind::Burn)] = {};
    p.status.effects[int(StatusKind::Chill)] = {};
    p.status.effects[int(StatusKind::Poison)] = {};
    Climate c = world::climateAt(int(std::floor(p.x / 16)), int(std::floor(p.y / 16)));
    int status = c.temp < -0.12f ? int(StatusKind::Chill) :
                 c.moist > 0.12f ? int(StatusKind::Poison) : int(StatusKind::Burn);
    pulse(p, p.x, p.bodyY(), 8, status);
  } else if (kind == 7 && delay > 0) {
    delay -= DT;
    if (delay <= 0) {
      p.heal(stored);
      stored = 0;
    }
  }
}

void ExpansionRelic::slay(Player &p, Enemy &e) {
  if (cd > 0)
    return;
  if (kind == 0 && p.level->streak > 0 && p.level->streak % 10 == 0) {
    cd = 3;
    pulse(p, e.x, e.bodyY(), 25, -1);
  } else if (kind == 3) {
    cd = 0.6f;
    pulse(p, e.x, e.bodyY(), 0, int(StatusKind::Poison));
  } else if (kind == 6 && e.elite && claims < 5) {
    cd = 20;
    claims++;
    p.level->add<Chest>(e.x, e.y);
  }
}

float ExpansionRelic::guard(Player &p, float damage) {
  if (cd > 0)
    return damage;
  if (kind == 1 && p.hp <= p.maxHp * 0.3f) {
    cd = 8;
    Spark::burst(*p.level, p.x, p.bodyY(), col::xp, 8, 70, true);
    return damage * 0.5f;
  }
  if (kind == 7) {
    cd = 8;
    delay = 3;
    stored = std::min(20.0f, damage * 0.4f);
  }
  return damage;
}

void ExpansionRelic::cast(Player &p) {
  if (kind != 4 || cd > 0)
    return;
  cd = 8;
  double a = std::atan2(p.yAim, p.xAim);
  for (int i = 0; i < 2; i++) {
    double angle = a + (i == 0 ? -0.35 : 0.35);
    p.level->add<ExpansionAttack>(3, p.x, p.bodyY(), std::cos(angle),
                                  std::sin(angle), 30 * p.might, 150.0f, 1)
        ->source = this;
  }
}
