#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/Level.hpp"

namespace {

Enemy *place(Level &level, double x, double y, EnemyKind kind) {
  auto made = Enemy::create(kind, 4, 1);
  Enemy *e = made.get();
  e->x = x, e->y = y;
  level.add(std::move(made));
  return e;
}

Recipe probe(unsigned payload) {
  return {"Probe", nullptr, "", "probe", 0, Form::Bolt, payload, 10, 1, 100, 100, 1,
          col::white, atlas::pt::Rune};
}

double distance(const Enemy &e, const Player &p) {
  return std::sqrt((e.x - p.x) * (e.x - p.x) + (e.y - p.y) * (e.y - p.y));
}

} // namespace

void testNewStatuses() {
  Status s;
  assert(s.apply(StatusKind::Bleed, 2, 4));
  assert(s.apply(StatusKind::Bleed, 2, 4));
  assert(std::abs(s.power(StatusKind::Bleed) - 8) < 1e-4f);
  float still = 0, moving = 0;
  Status a = s, b = s;
  b.moving = true;
  a.tick(0.5f, [&](float d, Upgrade *) { return still += d, true; });
  b.tick(0.5f, [&](float d, Upgrade *) { return moving += d, true; });
  assert(still > 0 && std::abs(moving - still * 1.6f) < 1e-3f);

  Status w;
  assert(w.apply(StatusKind::Weaken, 1, 0.9f));
  assert(std::abs(w.outgoing() - 0.5f) < 1e-4f);

  Status f;
  assert(f.apply(StatusKind::Fear, 1, 5) && f.afraid());
  f.tick(1.1f, [](float, Upgrade *) { return true; });
  assert(!f.afraid());
  std::cout << "[TEST] Bleed stacks and hurts more on the move, weaken caps, fear expires: PASS"
            << std::endl;
}

void testNewPayloads() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  assert(!p.afflict(StatusKind::Fear, 2, 1));

  Enemy *e = place(level, p.x + 60, p.y, EnemyKind::Knight);
  Enemy *weak = place(level, p.x - 120, p.y, EnemyKind::Rat);
  Enemy *boss = place(level, p.x, p.y + 140, EnemyKind::Rat);
  boss->boss = true;
  level.tick();
  level.tick();

  Recipe curse = probe(pay::Bleed | pay::Weaken | pay::Fear);
  RecipeShot *shot = level.add<RecipeShot>(curse, p.x, p.y, 1, 0, 10, 100);
  shot->hit(*e, 1, 0, 10, 0);
  shot->remove();
  assert(e->status.power(StatusKind::Bleed) > 0);
  assert(e->status.power(StatusKind::Weaken) > 0);
  assert(e->status.afraid());
  assert(e->power() < e->damageMul * e->might);
  double before = distance(*e, p);
  for (int i = 0; i < 30; i++)
    level.tick();
  assert(distance(*e, p) > before);

  Recipe shatter = probe(pay::Shatter);
  RecipeShot *cold = level.add<RecipeShot>(shatter, p.x, p.y, 1, 0, 10, 100);
  assert(e->afflict(StatusKind::Chill, 2, 0.3f));
  cold->hit(*e, 1, 0, 1, 0);
  cold->remove();
  assert(e->status.power(StatusKind::Chill) == 0);

  Recipe execute = probe(pay::Execute);
  RecipeShot *blade = level.add<RecipeShot>(execute, p.x, p.y, 1, 0, 10, 100);
  weak->hp = weak->maxHp * 0.05f;
  boss->hp = boss->maxHp * 0.05f;
  blade->hit(*weak, 1, 0, 0.001f, 0);
  blade->hit(*boss, 1, 0, 0.001f, 0);
  blade->remove();
  assert(weak->dead());
  assert(!boss->dead());
  game.level.reset();
  std::cout << "[TEST] Bleed, weaken, fear, shatter and execute payloads: PASS" << std::endl;
}

void testShapeForms() {
  auto arena = [](Form form, double dx, int count, int ticks, auto check) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.god = true;
    for (auto &up : p.upgrades)
      up->level = 0;
    Enemy *e = place(level, p.x + dx, p.y, EnemyKind::Knight);
    level.tick();
    level.tick();
    float hp = e->hp;
    Recipe r{"Probe", nullptr, "", "probe", 0, form, 0, 10, 1, 70, 160, count,
             col::white, atlas::pt::Rune};
    fireRecipe(r, p, nullptr, 10, 70, count);
    for (int i = 0; i < ticks; i++)
      level.tick();
    check(*e, p, hp);
    game.level.reset();
  };
  arena(Form::Whip, 45, 1, 20, [](Enemy &e, Player &, float hp) { assert(e.hp < hp); });
  arena(Form::Wall, 110, 1, 150, [](Enemy &e, Player &p, float hp) {
    assert(e.hp < hp);
    assert(e.x - p.x > 40);
  });
  arena(Form::Spiral, 50, 4, 90, [](Enemy &e, Player &, float hp) { assert(e.hp < hp); });
  std::cout << "[TEST] Whip cracks, walls hold foes back, spirals reach out: PASS" << std::endl;
}
