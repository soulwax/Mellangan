#include "entity/Player.hpp"

#include <cstring>
#include <limits>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "hero/Hero.hpp"
#include "particle/Spark.hpp"
#include "particle/TextParticle.hpp"
#include "particle/debris/FlameDebris.hpp"

Player::Player() : upgrades(Upgrade::all()) { hp = maxHp = 100; }

Player::~Player() = default;

void Player::onAdded() {
  Game &g = level->game;
  hero = g.dev && g.heroPick >= 0 && g.heroPick < HERO_COUNT
             ? HEROES[g.heroPick]
             : &heroOf(g.codex);
  hp = maxHp = g.balance.playerHp;
  xpBase = g.balance.xpBase, xpGrowth = g.balance.xpGrowth;
  hero->equip(*this);
  for (auto &u : upgrades)
    if (std::strcmp(u->name, hero->weapon) == 0)
      return u->levelUp(*this);
  upgrades.front()->levelUp(*this);
}

void Player::tick() {
  animT += DT;
  flash = 0;
  if (dead())
    return;

  status.tick(DT, [&](float damage, Upgrade *) {
    felledBy = "a lingering affliction";
    hurt(damage, true);
    return !dead();
  });
  if (dead())
    return;

  Input &in = level->game.input;
  double xd = in.right.down - in.left.down, yd = in.down.down - in.up.down;
  double dd = std::sqrt(xd * xd + yd * yd);
  if (dd > 0)
    xd /= dd, yd /= dd;
  int surface = level->surfaceAt(x, y);
  double k = damp(surface == 1 ? 6 : surface == 2 ? 2.5f : 18, DT);
  float v = speed * level->step * status.movement() *
            (surface == 1 ? 0.55f : surface == 2 ? 1.05f : 1);
  xa += (xd * v - xa) * k;
  ya += (yd * v - ya) * k;
  double px = x, py = y;
  x += xa * DT;
  y += ya * DT;
  level->collide(x, y, radius);
  double mx = x - px, my = y - py;
  level->distanceRan += float(std::sqrt(mx * mx + my * my));
  if (xd != 0)
    facing = xd > 0 ? 1.0f : -1.0f;
  if (xd != 0 || yd != 0)
    xAim = xd, yAim = yd;
  invuln = std::max(0.0f, invuln - DT);
  skillTime = std::max(0.0f, skillTime - DT);
  if (hero)
    hero->tick(*this);
  hp = std::min(maxHp, hp + regen * DT);

  for (auto &u : upgrades)
    if (u->level > 0)
      u->tick(*this);
  if (rng.chance(0.7f))
    FlameDebris::lick(*level, torchX(), y + 0.5, 12, 1);
}

void Player::render(Screen &s) {
  if (dead())
    return;
  bool blink = invuln > 0 && std::fmod(invuln, 0.12f) < 0.06f;
  float move = float(std::sqrt(xa * xa + ya * ya) / speed);
  drawBody(s, xp, yp, hero ? hero->sprite() : atlas::tile(tile), false, move, col::white,
           blink ? 0.7f : 0.0f, {1, 0.25f, 0.25f, 1});
  int xt = xp + int(facing * 6);
  s.rect(xt, yp - 10, 1, 7, {0.42f, 0.26f, 0.13f, 1});
  s.rect(xt - 1, yp - 12, 3, 2, {0.22f, 0.13f, 0.08f, 1});
  if (hp < maxHp) {
    s.rect(xp - 8, yp + 2, 16, 2, {0.1f, 0.02f, 0.04f, 0.9f});
    s.rect(xp - 8, yp + 2, 16 * hp / maxHp, 2, col::red);
  }
}

void Player::renderShadow(Screen &s) {
  if (!dead())
    drawShadow(s, xp, yp, false);
}

void Player::renderLight(Screen &s) {
  int yb = yp - 7;
  if (dead())
    return s.glow(xp, yb, 200, {0.25f, 0.05f, 0.05f, 1});
  float f = flicker(level->time, 0),
        reach = 1 + 0.08f * std::min(level->night.lit, 8);
  int xt = xp + int(facing * 6), yt = yp - 13;
  Color warm{0.34f * f, 0.27f * f * f, 0.17f * f * f, 1},
      core{0.22f * f, 0.16f * f * f, 0.08f * f, 1};
  s.glow(xt, yt, 300 * reach * (0.97f + 0.03f * f), warm);
  s.glow(xt, yt, 110 * reach * f, core);
  s.glow(xt, yt, 22 * f, {0.35f * f, 0.2f * f, 0.05f, 1});
  level->lights.add(torchX(), y, 200 * reach, f);
}

void Player::hurt(float damage, bool periodic) {
  if ((!periodic && invuln > 0) || dead() || god)
    return;
  if (!periodic)
    damage *= status.incoming() * level->game.balance.damageTaken;
  if (hero && (damage = hero->guard(*this, damage)) <= 0)
    return;
  for (auto &up : upgrades)
    if (up->level > 0 && (damage = up->guard(*this, damage)) <= 0)
      return;
  damage = std::max(periodic ? 0.1f : 1.0f, damage - armor);
  hp -= damage;
  if (!periodic)
    invuln = level->game.balance.invuln, level->game.hitStop(0.05f);
  level->game.onPlayerHurt();
  Sound::hurt.play(0.7f);
  TextParticle::pop(*level, x, bodyY(), int(std::lround(damage)), col::red);
  Spark::burst(*level, x, bodyY(), col::red, 6, 60, false);
  if (hp > 0)
    return;
  if (revives > 0)
    return rise();
  hp = 0;
  status.clear();
  Spark::burst(*level, x, bodyY(), {0.7f, 0.4f, 1, 1}, 40, 120, true);
  level->game.onPlayerDied();
}

void Player::cast() {
  if (!hero || dead() || skillTime > 0)
    return;
  hero->cast(*this);
  skillTime = hero->cooldown * haste;
  for (auto &up : upgrades)
    if (up->level > 0)
      up->cast(*this);
}

void Player::rise() {
  status.clear();
  revives--;
  hp = maxHp, invuln = 3;
  level->enemiesNear(x, y, 180, [&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y;
    if (xd * xd + yd * yd < 160 * 160)
      e.hurt(400 * might, xd, yd, 260);
  });
  Spark::burst(*level, x, bodyY(), {1, 0.55f, 0.2f, 1}, 60, 240, true);
  for (int i = 0; i < 10; i++)
    level->add<FlameDebris>(x, y, 8);
  level->game.onExplosion();
  level->game.announce("THE PHOENIX RISES");
  Sound::nova.play(0.8f, 0);
}

void Player::heal(float amount) {
  hp = std::min(maxHp, hp + amount);
  TextParticle::pop(*level, x, bodyY(), int(amount), col::heal);
}

void Player::gainXp(float amount) {
  if (hero)
    amount *= hero->wisdom();
  exp += amount;
  while (exp >= expNext) {
    exp -= expNext;
    lvl++, pendingLevels++;
    double next = (xpBase + lvl * xpBase) * std::pow(double(xpGrowth), lvl - 2);
    expNext = float(std::min(next, double(std::numeric_limits<float>::max())));
  }
}
