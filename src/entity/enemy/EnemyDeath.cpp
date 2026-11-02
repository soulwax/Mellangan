#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/Ability.hpp"

#include "Game.hpp"
#include <spdlog/fmt/fmt.h>
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/AlphaWolf.hpp"
#include "entity/enemy/Burrower.hpp"
#include "entity/enemy/Cyclops.hpp"
#include "entity/enemy/Ghost.hpp"
#include "entity/enemy/Mimic.hpp"
#include "entity/enemy/Necromancer.hpp"
#include "entity/enemy/Renegade.hpp"
#include "entity/enemy/Thief.hpp"
#include "entity/enemy/ExpansionEnemy.hpp"
#include "entity/enemy/Wolf.hpp"
#include "entity/enemy/ai/Plans.hpp"
#include "entity/enemy/ai/Melee.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/pickup/Potion.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "entity/projectile/Orb.hpp"
#include "level/Level.hpp"
#include "particle/TextParticle.hpp"
#include "particle/DeathEffect.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/debris/Chip.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"

namespace {
constexpr int ELITE_XP = 6;

void puffs(Level &level, double x, double y, Color c, int n, float speed,
           float rise) {
  for (int i = 0; i < n && Drift::room(); i++) {
    float a = rng.range(0, TAU), v = rng.range(0.4f, 1) * speed;
    Drift *d = level.add<Drift>(x, y, std::cos(a) * v, std::sin(a) * v * 0.6f - rise,
                                atlas::pt::Puff, c, rng.range(0.6f, 1.0f));
    d->grow = 1.3f, d->scale = 0.6f, d->spin = rng.range(-1, 1);
  }
}

void chips(Level &level, double x, double y, float z, int icon, Color c, int n,
           double speed) {
  for (int i = 0; i < n && Debris::room(); i++)
    level.add<Chip>(x, y, z, icon, c, speed);
}

bool signature(Level &level, EnemyKind k, double x, double y, double body,
               float scale) {
  switch (k) {
  case EnemyKind::Skeleton:
    chips(level, x, y, 8 * scale, atlas::pt::Bone, col::white, 5, 1.1);
    puffs(level, x, y, Color{0.85f, 0.82f, 0.72f, 0.45f}, 3, 20, 4);
    return true;
  case EnemyKind::Iceback:
    chips(level, x, y, 8 * scale, atlas::pt::IceShard, col::white, 9, 1.4);
    level.add<Shockwave>(x, body, 3, 18 * scale, 0.3f,
                         Color{0.75f, 0.92f, 1, 0.9f}, false);
    return true;
  case EnemyKind::Sporeling:
  case EnemyKind::MycelialBrute:
    puffs(level, x, body, Color{0.78f, 0.95f, 0.55f, 0.5f},
          k == EnemyKind::Sporeling ? 5 : 9, 35 * scale, 6);
    return true;
  case EnemyKind::Mimic:
    chips(level, x, y, 6, atlas::pt::Coin, col::white, 8, 1.4);
    chips(level, x, y, 6, atlas::pt::Pebble, Color{0.7f, 0.45f, 0.25f, 1}, 5, 1.2);
    return true;
  case EnemyKind::Burrower:
    chips(level, x, y, 4, atlas::pt::Rock, col::white, 4, 1.3);
    chips(level, x, y, 4, atlas::pt::Pebble, col::white, 4, 1.6);
    puffs(level, x, y, Color{0.62f, 0.5f, 0.38f, 0.5f}, 4, 30, 2);
    return true;
  default:
    return false;
  }
}

void flesh(Level &level, EnemyKind k, double x, double y, double body,
           float scale) {
  switch (k) {
  case EnemyKind::Knight:
    chips(level, x, y, 8 * scale, atlas::pt::Pebble, Color{0.8f, 0.85f, 0.95f, 1},
          4, 1.2);
    Spark::burst(level, x, body, col::white, 5, 80, false);
    break;
  case EnemyKind::Cultist:
  case EnemyKind::Necromancer:
    puffs(level, x, body, Color{0.35f, 0.15f, 0.45f, 0.55f}, 5, 25, 10);
    for (int i = 0; i < 2 && Drift::room(); i++) {
      Drift *d = level.add<Drift>(x, body, rng.range(-8, 8), -30,
                                  atlas::pt::Wisp, Color{0.75f, 0.55f, 1, 0.9f},
                                  0.9f);
      d->sway = 6, d->drag = 0.5f;
    }
    break;
  case EnemyKind::Thief:
    chips(level, x, y, 8, atlas::pt::Coin, col::white, 6, 1.3);
    break;
  case EnemyKind::MireLeech:
    puffs(level, x, y, Color{0.35f, 0.45f, 0.25f, 0.5f}, 3, 20, 2);
    break;
  default:
    break;
  }
}
} // namespace

namespace {

Source lootSource(const Enemy &e) {
  if (e.boss)
    return Source::Boss;
  if (e.title)
    return Source::Champion;
  if (e.is(EnemyKind::Mimic))
    return Source::Mimic;
  if (e.elite)
    return Source::Elite;
  return e.type.hp < 25 ? Source::Swarm : e.type.hp < 80 ? Source::Line : Source::Brute;
}

int monsterLevel(const Enemy &e) {
  const RunRules &r = e.level->rules;
  float t = r.threat ? r.threat(e.x, e.y) : e.level->time;
  int bonus = e.boss ? 5 : e.title ? 3 : e.elite ? 2 : 0;
  return std::clamp(1 + int(t / 15) + bonus, 1, 99);
}

} // namespace

void Enemy::die() {
  status.clear();
  for (auto &up : upgrades)
    up->holder = nullptr, level->retired.push_back(std::move(up));
  upgrades.clear();
  remove();
  level->kills++;
  level->slain[kind()]++;
  level->streak++, level->streakTime = 2.5f;
  level->bestStreak = std::max(level->bestStreak, level->streak);
  level->game.codex.slain[kind()]++;
  if (Player *p = level->player; p->hero) {
    p->hero->slay(*p, *this);
    for (auto &up : p->upgrades)
      if (up->level > 0)
        up->slay(*p, *this);
  }
  Sound::kill.play(0.35f, 0.12f);
  Player &killer = *level->player;
  if (killer.bonus.bloodthirst > 0 && !killer.dead())
    killer.hp = std::min(killer.maxHp, killer.hp + killer.bonus.bloodthirst);
  if (float burn = status.power(StatusKind::Burn);
      burn > 0 && killer.bonus.pyro > 0 && rng.chance(killer.bonus.pyro / 100)) {
    for (Enemy *o : level->nearest(x, y, 60, 4))
      if (o != this)
        o->afflict(StatusKind::Burn, 2, burn);
    level->add<Shockwave>(x, bodyY(), 4, 60, 0.3f, Color{1, 0.5f, 0.2f, 0.8f}, false);
  }
  int gems = 1;
  for (const Gild &g : level->gilds)
    if (g.until > level->time && (x - g.x) * (x - g.x) + (y - g.y) * (y - g.y) < g.r * g.r)
      gems = 2;
  for (int i = 0; i < gems; i++)
    Gem::drop(*level, x + i * 6, y - 3,
              type.xp * (elite ? ELITE_XP : 1) * level->xpMul *
                  (1 + std::min(level->streak, 100) * 0.005f) *
                  (1 + killer.bonus.scavenge / 100));
  if (elite && rng.chance(0.25f))
    level->add<Potion>(x + 6, y);
  if (level->rules.loot)
    for (int i = 0; i <= (title && killer.bonus.extraLoot > 0); i++)
      ItemDrop::spill(*level, x, y, lootSource(*this), monsterLevel(*this));
  if (elite)
    level->game.hitStop(0.035f);
  traits.died(*this, type.damage * damageMul * 2);
  if (boss) {
    level->add<Chest>(x, y);
    level->add<Shockwave>(x, bodyY(), 6, 90, 0.6f, Color{0.95f, 0.2f, 0.25f, 1})
        ->spokes = 16;
    Spark::burst(*level, x, bodyY(), type.blood, 30, 150, true);
    level->game.hitStop(0.12f);
    level->game.onExplosion();
    level->game.announce(def ? def->fall
                             : is(EnemyKind::Alpha) ? "THE PACK LORD FALLS"
                                                    : "THE LICH CRUMBLES");
    for (int i = 0; i < 8; i++)
      Gem::drop(*level, x + rng.range(-20, 20), y + rng.range(-14, 14),
                30 * level->xpMul);
  }
  if (title && !def) {
    level->add<Chest>(x, y);
    level->add<Shockwave>(x, bodyY(), 6, 70, 0.5f, col::gold.alpha(0.9f));
    Spark::burst(*level, x, bodyY(), col::gold, 24, 130, true);
    level->game.hitStop(0.08f);
    level->game.announce(fmt::format("{} IS SLAIN", title));
  }
  if (rng.chance(1 / 150.0f))
    level->add<Potion>(x, y);
  if (gilded) {
    constexpr Color GOLD{1, 0.7f, 0.15f, 1};
    level->add<DeathEffect>(x, y, float(z), sprite(), scale, facing, GOLD,
                            DeathStyle::Gilded);
    level->add<Shockwave>(x, bodyY(), 4, 22 * scale, 0.3f, GOLD, false);
    for (int i = 0; i < 7 && Debris::room(); i++)
      level->add<Chip>(x, y, 6 * scale, atlas::pt::Coin, col::white, 1.3);
    return;
  }
  EnemyKind k = EnemyKind(kind());
  bool spirit = k == EnemyKind::Ghost || k == EnemyKind::BogLantern ||
                k == EnemyKind::FrostWisp;
  bool flame = k == EnemyKind::Demon || k == EnemyKind::EmberImp ||
               k == EnemyKind::CinderGolem;
  bool slime = k == EnemyKind::Slime;
  bool fallen = k == EnemyKind::Bat || k == EnemyKind::Scorpion ||
                k == EnemyKind::Spider || k == EnemyKind::GlassMantis ||
                k == EnemyKind::DustVulture;
  if (spirit || flame || slime || fallen) {
    DeathStyle style = spirit ? DeathStyle::Spirit : flame ? DeathStyle::Flame :
                       slime ? DeathStyle::Slime : DeathStyle::Fallen;
    if (DeathEffect::alive < 180)
      level->add<DeathEffect>(x, y, float(z), sprite(), scale, facing,
                              type.blood, style);
    if (spirit || slime) {
      for (int i = 0; i < 12 && Spark::alive < 500; i++)
        level->add<Spark>(x + rng.range(-6, 6) * scale, bodyY(),
                          rng.range(-15, 15), spirit ? -rng.range(25, 55) : rng.range(0, 8),
                          type.blood, rng.range(0.4f, 0.9f), rng.range(0.3f, 0.7f), false);
    } else if (flame) {
      for (int i = 0; i < 12; i++)
        FlameDebris::lick(*level, x + rng.range(-6, 6) * scale,
                          y + rng.range(-4, 4) * scale, rng.range(2, 12) * scale, 1);
      Spark::burst(*level, x, bodyY(), {1, 0.35f, 0.08f, 1}, 12, 70, true);
    }
  } else if (signature(*level, k, x, y, bodyY(), scale)) {
    if (DeathEffect::alive < 180 && k != EnemyKind::Sporeling &&
        k != EnemyKind::Iceback)
      level->add<DeathEffect>(x, y, float(z), sprite(), scale, facing,
                              type.blood, DeathStyle::Fallen);
  } else {
    flesh(*level, k, x, y, bodyY(), scale);
    Color blood = k == EnemyKind::Rat || k == EnemyKind::Cultist ||
                          k == EnemyKind::Knight || k == EnemyKind::Cyclops ||
                          k == EnemyKind::Wolf || k == EnemyKind::Alpha ||
                          k == EnemyKind::Necromancer || k == EnemyKind::Thief
                      ? Color{0.65f, 0.08f, 0.1f, 1}
                      : type.blood;
    Debris::gore(*level, x, y, 7 * scale, blood, true);
    Spark::burst(*level, x, bodyY(), blood, 6, 45, false);
  }
}
