#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "entity/projectile/Arrow.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

namespace {

struct Arena {
  Game game;
  Level &level;
  Player &p;

  Arena() : level(*(game.level = std::make_unique<Level>(game))), p(*level.player) {
    p.god = true;
    for (auto &up : p.upgrades)
      up->level = 0;
  }
  ~Arena() { game.level.reset(); }

  WildFoe *add(EnemyKind kind, double dx, double dy) {
    auto made = Enemy::create(kind, 4, 1);
    auto *e = dynamic_cast<WildFoe *>(made.get());
    assert(e);
    e->x = p.x + dx, e->y = p.y + dy;
    level.add(std::move(made));
    return e;
  }

  void run(int ticks) {
    for (int i = 0; i < ticks; i++)
      level.tick();
  }

  template <class T> int particles() {
    int n = 0;
    for (auto &q : level.particles)
      n += !q->removed && dynamic_cast<T *>(q.get());
    return n;
  }

  int hostileArrows() {
    int n = 0;
    for (auto [id, o] : level.reg.view<Owned>().each())
      if (auto *a = dynamic_cast<Arrow *>(o.self.get()); a && !a->removed && a->hostile)
        n++;
    return n;
  }
};

} // namespace

void testGreenwoodFoes() {
  for (int k = int(WildFoe::FIRST); k <= int(WildFoe::LAST); k++) {
    Arena a;
    WildFoe *e = a.add(EnemyKind(k), 70, 20);
    assert(e->sprite().w == 16 && e->name() && e->plan);
    a.run(240);
  }
  {
    Arena a;
    WildFoe *shaman = a.add(EnemyKind::GoblinShaman, 120, 0);
    WildFoe *archer = a.add(EnemyKind::GoblinArcher, 150, 0);
    a.run(2);
    archer->hp = archer->maxHp * 0.4f;
    float hp = archer->hp;
    a.run(150);
    assert(!shaman->removed && archer->hp > hp);
  }
  {
    Arena a;
    WildFoe *sapling = a.add(EnemyKind::Sapling, 90, 0);
    a.run(2);
    sapling->life = sapling->wild.grow - DT * 0.5f;
    a.run(3);
    assert(sapling->removed && a.level.count<Enemy>() >= 1);
    bool treant = false;
    a.level.each<Enemy>([&](Enemy &e) { treant |= e.is(EnemyKind::Treant); });
    assert(treant);
  }
  {
    Arena a;
    a.add(EnemyKind::MothWisp, 0, 0);
    a.run(3);
    assert(a.p.dim > 0);
  }
  {
    Arena a;
    WildFoe *knife = a.add(EnemyKind::Cutthroat, -20, 0);
    a.p.facing = 1;
    assert(knife->contactBonus(a.p) == 1.5f);
    knife->x = a.p.x + 20;
    assert(knife->contactBonus(a.p) == 1.0f);
  }
  {
    Arena a;
    WildFoe *thorn = a.add(EnemyKind::Thornling, 50, 0);
    a.run(2);
    thorn->hurt(1e6f, 0, 0, 0);
    a.run(1);
    assert(a.particles<Bramble>() == 1);
  }
  {
    Arena a;
    a.add(EnemyKind::GoblinArcher, 140, 0);
    int seen = 0;
    for (int i = 0; i < 400 && !seen; i++)
      a.run(1), seen = a.hostileArrows();
    assert(seen >= 1);
  }
  std::cout << "[TEST] Greenwood foes: shamans mend, saplings grow, moths dim, cutthroats "
               "backstab, thornlings leave brambles, archers loose volleys: PASS"
            << std::endl;
}
