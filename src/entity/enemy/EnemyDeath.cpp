#include "entity/enemy/Enemy.hpp"
#include "gui/Lang.hpp"
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
#include "entity/enemy/Named.hpp"
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

Lean leanOf(const Enemy &e) {
  using K = EnemyKind;
  static constexpr std::pair<K, Lean> TIES[] = {
      {K::GoblinArcher, Lean::Gold},   {K::GoblinShaman, Lean::Gold},
      {K::CinderGolem, Lean::Ore},     {K::CrystalGolem, Lean::Ore},
      {K::SaltGolem, Lean::Ore},       {K::OreGolem, Lean::Ore},
      {K::KoboldMiner, Lean::Ore},     {K::HollowSentinel, Lean::Ore},
      {K::Cultist, Lean::Arcane},      {K::Necromancer, Lean::Arcane},
      {K::SunkenPriest, Lean::Arcane}, {K::TotemShaman, Lean::Arcane},
      {K::FlameCultist, Lean::Arcane}, {K::Ghost, Lean::Charm},
      {K::IceWraith, Lean::Charm},     {K::AshWraith, Lean::Charm},
      {K::BarrowWight, Lean::Charm},   {K::Boar, Lean::Hide},
      {K::Hyena, Lean::Hide},          {K::Lynx, Lean::Hide},
      {K::MoonHound, Lean::Hide},      {K::Wolf, Lean::Hide},
      {K::Alpha, Lean::Hide},          {K::Yeti, Lean::Hide},
      {K::Mimic, Lean::Riches},        {K::GraveRobber, Lean::Riches},
      {K::Thief, Lean::Riches},
  };
  for (auto [k, lean] : TIES)
    if (e.is(k))
      return lean;
  return Lean::None;
}

int monsterLevel(const Enemy &e) {
  int bonus = e.boss ? 5 : e.title ? 3 : e.elite ? 2 : 0;
  return std::clamp(e.level->monsterLevel(e.x, e.y) + bonus, 1, 99);
}

void tally(Enemy &e) {
  Level &l = *e.level;
  l.kills++;
  l.slain[e.kind()]++;
  l.streak++, l.streakTime = 2.5f;
  l.bestStreak = std::max(l.bestStreak, l.streak);
  l.game.codex.slain[e.kind()]++;
  if (Player *p = l.player; p->hero) {
    p->hero->slay(*p, e);
    for (auto &up : p->upgrades)
      if (up->level > 0)
        up->slay(*p, e);
  }
}

void onKill(Enemy &e, Player &killer) {
  const ItemStats &b = killer.bonus();
  if (b.bloodthirst > 0 && !killer.dead())
    killer.hp = std::min(killer.maxHp, killer.hp + b.bloodthirst);
  if (float burn = e.status.power(StatusKind::Burn);
      burn > 0 && b.pyro > 0 && rng.chance(b.pyro / 100)) {
    for (Enemy *o : e.level->nearest(e.x, e.y, 60, 4))
      if (o != &e)
        o->afflict(StatusKind::Burn, 2, burn);
    e.level->add<Shockwave>(e.x, e.bodyY(), 4, 60, 0.3f, Color{1, 0.5f, 0.2f, 0.8f}, false);
  }
}

void spoils(Enemy &e, const Player &killer) {
  Level &l = *e.level;
  int gems = 1;
  for (const Gild &g : l.gilds)
    if (g.until > l.time && (e.x - g.x) * (e.x - g.x) + (e.y - g.y) * (e.y - g.y) < g.r * g.r)
      gems = 2;
  for (int i = 0; i < gems; i++)
    Gem::drop(l, e.x + i * 6, e.y - 3,
              e.type.xp * (e.elite ? ELITE_XP : 1) * l.xpMul *
                  (1 + std::min(l.streak, 100) * 0.005f) * (1 + killer.bonus().scavenge / 100));
  if (e.elite && rng.chance(0.25f))
    l.add<Potion>(e.x + 6, e.y);
  if (l.rules.loot)
    for (int i = 0, n = e.title && killer.bonus().extraLoot > 0; i <= n; i++)
      ItemDrop::spill(l, e.x, e.y, lootSource(e), monsterLevel(e), leanOf(e));
}

void bossFalls(Enemy &e) {
  Level &l = *e.level;
  l.add<Chest>(e.x, e.y);
  l.add<Shockwave>(e.x, e.bodyY(), 6, 90, 0.6f, Color{0.95f, 0.2f, 0.25f, 1})->spokes = 16;
  Spark::burst(l, e.x, e.bodyY(), e.type.blood, 30, 150, true);
  l.game.hitStop(0.12f);
  l.game.onExplosion();
  l.game.announce(e.def ? e.def->fall
                  : e.is(EnemyKind::Alpha) ? "THE PACK LORD FALLS"
                                           : "THE LICH CRUMBLES");
  for (int i = 0; i < 8; i++)
    Gem::drop(l, e.x + rng.range(-20, 20), e.y + rng.range(-14, 14), 30 * l.xpMul);
}

void championFalls(Enemy &e) {
  Level &l = *e.level;
  l.add<Chest>(e.x, e.y);
  l.add<Shockwave>(e.x, e.bodyY(), 6, 70, 0.5f, col::gold.alpha(0.9f));
  Spark::burst(l, e.x, e.bodyY(), col::gold, 24, 130, true);
  l.game.hitStop(0.08f);
  l.game.announce(say(lang.isSlain, tr(e.title)));
}

enum class Remains { Gore, Spirit, Flame, Slime, Fallen };

Remains remainsOf(EnemyKind k) {
  using K = EnemyKind;
  switch (k) {
  case K::Ghost:
  case K::BogLantern:
  case K::FrostWisp:
    return Remains::Spirit;
  case K::Demon:
  case K::EmberImp:
  case K::CinderGolem:
    return Remains::Flame;
  case K::Slime:
    return Remains::Slime;
  case K::Bat:
  case K::Scorpion:
  case K::Spider:
  case K::GlassMantis:
  case K::DustVulture:
    return Remains::Fallen;
  default:
    return Remains::Gore;
  }
}

bool redBlooded(EnemyKind k) {
  using K = EnemyKind;
  switch (k) {
  case K::Rat:
  case K::Cultist:
  case K::Knight:
  case K::Cyclops:
  case K::Wolf:
  case K::Alpha:
  case K::Necromancer:
  case K::Thief:
    return true;
  default:
    return false;
  }
}

void gilt(Enemy &e) {
  constexpr Color GOLD{1, 0.7f, 0.15f, 1};
  Level &l = *e.level;
  l.add<DeathEffect>(e.x, e.y, float(e.z), e.sprite(), e.scale, e.facing, GOLD, DeathStyle::Gilded);
  l.add<Shockwave>(e.x, e.bodyY(), 4, 22 * e.scale, 0.3f, GOLD, false);
  for (int i = 0; i < 7 && Debris::room(); i++)
    l.add<Chip>(e.x, e.y, 6 * e.scale, atlas::pt::Coin, col::white, 1.3);
}

void remains(Enemy &e) {
  Level &l = *e.level;
  EnemyKind k = EnemyKind(e.kind());
  double x = e.x, y = e.y, body = e.bodyY();
  float scale = e.scale;
  Remains r = remainsOf(k);
  if (r != Remains::Gore) {
    constexpr DeathStyle STYLE[] = {DeathStyle::Fallen, DeathStyle::Spirit, DeathStyle::Flame,
                                    DeathStyle::Slime, DeathStyle::Fallen};
    if (DeathEffect::alive < 180)
      l.add<DeathEffect>(x, y, float(e.z), e.sprite(), scale, e.facing, e.type.blood, STYLE[int(r)]);
    if (r == Remains::Spirit || r == Remains::Slime) {
      for (int i = 0; i < 12 && Spark::alive < 500; i++)
        l.add<Spark>(x + rng.range(-6, 6) * scale, body, rng.range(-15, 15),
                     r == Remains::Spirit ? -rng.range(25, 55) : rng.range(0, 8), e.type.blood,
                     rng.range(0.4f, 0.9f), rng.range(0.3f, 0.7f), false);
    } else if (r == Remains::Flame) {
      for (int i = 0; i < 12; i++)
        FlameDebris::lick(l, x + rng.range(-6, 6) * scale, y + rng.range(-4, 4) * scale,
                          rng.range(2, 12) * scale, 1);
      Spark::burst(l, x, body, {1, 0.35f, 0.08f, 1}, 12, 70, true);
    }
  } else if (signature(l, k, x, y, body, scale)) {
    if (DeathEffect::alive < 180 && k != EnemyKind::Sporeling && k != EnemyKind::Iceback)
      l.add<DeathEffect>(x, y, float(e.z), e.sprite(), scale, e.facing, e.type.blood,
                         DeathStyle::Fallen);
  } else {
    flesh(l, k, x, y, body, scale);
    Color blood = redBlooded(k) ? Color{0.65f, 0.08f, 0.1f, 1} : e.type.blood;
    Debris::gore(l, x, y, 7 * scale, blood, true);
    Spark::burst(l, x, body, blood, 6, 45, false);
  }
}

} // namespace

void Enemy::die() {
  status.clear();
  for (auto &up : upgrades)
    up->holder = nullptr, level->retired.push_back(std::move(up));
  upgrades.clear();
  remove();
  if (!garrison)
    tally(*this);
  Sound::kill.play(0.35f, 0.12f);
  Player &killer = *level->player;
  onKill(*this, killer);
  if (!garrison)
    spoils(*this, killer);
  if (pack)
    named::fall(*this);
  if (elite)
    level->game.hitStop(0.035f);
  traits.died(*this, type.damage * damageMul * 2);
  if (boss)
    bossFalls(*this);
  if (title && !def)
    championFalls(*this);
  if (rng.chance(1 / 150.0f))
    level->add<Potion>(x, y);
  if (gilded)
    gilt(*this);
  else
    remains(*this);
}
