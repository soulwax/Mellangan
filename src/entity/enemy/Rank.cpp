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
const Cast SKILLS[] = {
    {[](Enemy &e) { ability::warcry(e, 110, 4); }, 9, 110, {1, 0.4f, 0.3f, 1}},
    {[](Enemy &e) { ability::quake(e, 80, 14); }, 8, 80, {0.85f, 0.65f, 0.4f, 1}},
    {[](Enemy &e) { ability::snare(e, 110, 4); }, 10, 110, {0.85f, 0.78f, 0.6f, 1}},
    {[](Enemy &e) { ability::tumble(e, 70); }, 6, 220, {0.8f, 0.85f, 1, 1}},
    {[](Enemy &e) { ability::firering(e, 6, 5); }, 11, 90, {1, 0.5f, 0.15f, 1}},
    {[](Enemy &e) { ability::whirlwind(e, 56, 12); }, 7, 56, {0.85f, 0.9f, 1, 1}},
    {[](Enemy &e) { ability::bandage(e, e.maxHp * 0.2f); }, 12, 240,
     {0.5f, 1, 0.6f, 1}},
    {[](Enemy &e) { ability::miasma(e, 90, 3, 4); }, 10, 90, {0.55f, 0.95f, 0.35f, 1}},
    {[](Enemy &e) { ability::banner(e, 80, 6); }, 14, 160, {1, 0.85f, 0.45f, 1}},
    {[](Enemy &e) { ability::hunt(e, 1, 6); }, 7, 220, {0.85f, 1, 0.55f, 1}},
    {[](Enemy &e) { ability::drain(e, 80, 8); }, 9, 80, {0.85f, 0.2f, 0.3f, 1}},
    {[](Enemy &e) { ability::overgrowth(e, 70, 5); }, 12, 90, {0.45f, 0.85f, 0.35f, 1}},
    {[](Enemy &e) { ability::eclipse(e, 70, 5); }, 12, 120, {0.6f, 0.5f, 0.95f, 1}},
};
} // namespace

void Enemy::makeElite(float health) {
  elite = true;
  traits.add(Trait(int(roll() * int(Trait::Count)) % int(Trait::Count)));
  hp = maxHp *= health;
  scale *= 1.35f;
  radius *= 1.3f;
}

void Enemy::crown(const char *name) {
  title = name;
  if (!elite)
    makeElite(8);
  else
    hp = maxHp *= 2;
  for (int i = 0; i < 8; i++) {
    Trait t = Trait(int(roll() * int(Trait::Count)) % int(Trait::Count));
    if (!traits.has(t)) {
      traits.add(t);
      break;
    }
  }
  learn();
  scale *= 1.15f, radius *= 1.1f;
  aware = true;
}

void Enemy::rule(const BossDef &d) {
  def = &d, title = d.title, phase = 0;
  casts = d.phases[0].casts, castCount = d.phases[0].count;
  castTime = 4;
}

void Enemy::enrage() {
  phase++;
  casts = def->phases[phase].casts, castCount = def->phases[phase].count;
  castTime = 1.2f, wind = 0, winding = nullptr, frenzy = 2;
  shield(2);
  for (int i = 0, n = def->minions ? def->minions + phase : 0; i < n; i++) {
    double a = TAU * i / n;
    level->spawner.spawn(def->minion, x + std::cos(a) * 40, y + std::sin(a) * 40);
  }
  level->add<Shockwave>(x, bodyY(), 8, 90, 0.5f, def->tone);
  Spark::burst(*level, x, bodyY(), def->tone, 24, 140, true);
  level->game.hitStop(0.06f);
  level->game.announce(fmt::format("{} ENRAGES", def->title));
  Sound::boss.play(0.6f, 0);
}

void Enemy::learn() {
  if (castCount)
    return;
  casts = &SKILLS[int(roll() * std::size(SKILLS)) % int(std::size(SKILLS))];
  castCount = 1;
  castTime = casts->every * (0.3f + 0.4f * roll());
}

void Enemy::channel() {
  if (wind > 0) {
    if ((wind -= DT) <= 0 && winding)
      winding->fn(*this);
    return;
  }
  if ((castTime -= DT) > 0)
    return;
  const Cast &c = casts[castCount > 1 ? int(roll() * castCount) % castCount : 0];
  const Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y;
  if (!aware || p.dead() || xd * xd + yd * yd > c.range * c.range) {
    castTime = 0.4f;
    return;
  }
  winding = &c, wind = boss ? WIND * 1.4f : WIND;
  castTime = c.every * (0.85f + 0.35f * roll());
}

void Enemy::aim(double &ax, double &ay) const {
  const Player &p = *level->player;
  double xd = p.x - x, yd = p.y - y, d = std::max(1e-3, std::sqrt(xd * xd + yd * yd));
  ax = xd / d, ay = yd / d;
}

void Enemy::split() {
  for (int i = 0; i < 2; i++) {
    auto e = Enemy::create(EnemyKind(kind()), 1, damageMul);
    e->hp = e->maxHp = maxHp / 8;
    e->x = x + (i ? 8 : -8), e->y = y, e->aware = true;
    level->add(std::move(e));
  }
  Spark::burst(*level, x, bodyY(), TRAITS[int(Trait::Splitting)].color, 8, 60, false);
}

void Enemy::makeBoss(float health, float damage) {
  boss = true;
  hp = maxHp = health;
  damageMul *= damage;
  scale *= 2.2f;
  radius *= 1.9f;
}
