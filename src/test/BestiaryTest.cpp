#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Orb.hpp"
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

  Enemy *any(EnemyKind kind, double dx, double dy, Trait trait) {
    auto made = Enemy::create(kind, 4, 1);
    Enemy *e = made.get();
    e->x = p.x + dx, e->y = p.y + dy;
    e->traits.add(trait);
    level.add(std::move(made));
    return e;
  }

  int orbs() {
    int n = 0;
    for (auto [id, o] : level.reg.view<Owned>().each())
      n += dynamic_cast<Orb *>(o.self.get()) && !o.self->removed;
    return n;
  }

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

void testChampionTraits() {
  auto contact = [](Trait trait, auto check) {
    Arena a;
    a.p.god = false, a.p.hp = a.p.maxHp = 1e6f, a.p.invuln = 0;
    a.any(EnemyKind::Knight, 3, 0, trait);
    float skill = a.p.skillTime;
    for (int i = 0; i < 30 && a.p.hp >= a.p.maxHp; i++)
      a.run(1);
    assert(a.p.hp < a.p.maxHp);
    check(a.p, skill);
  };
  contact(Trait::Cursed, [](Player &p, float) { assert(p.status.power(StatusKind::Weaken) > 0); });
  contact(Trait::Silencing, [](Player &p, float skill) { assert(p.skillTime >= skill + 1.9f); });
  contact(Trait::FireEnchanted, [](Player &p, float) { assert(p.status.power(StatusKind::Burn) > 0); });
  {
    Arena a;
    a.p.god = false, a.p.hp = a.p.maxHp = 1e6f;
    Enemy *e = a.any(EnemyKind::Rat, 30, 0, Trait::FireEnchanted);
    a.run(2);
    a.p.status.clear();
    e->hurt(1e6f, 0, 0, 0);
    assert(a.p.status.power(StatusKind::Burn) > 0);
  }
  {
    Arena a;
    Enemy *e = a.any(EnemyKind::Knight, 200, 0, Trait::StoneSkin);
    a.run(2);
    assert(std::abs(e->traits.incoming(*e, 100) - 65) < 1e-3f);
    assert(e->traits.speed() < 1);
  }
  {
    Arena a;
    a.any(EnemyKind::Cultist, 120, 0, Trait::Multishot);
    int most = 0;
    for (int i = 0; i < 600 && most < 3; i++)
      a.run(1), most = std::max(most, a.orbs());
    assert(most >= 3);
  }
  {
    Arena a;
    a.any(EnemyKind::Knight, 150, 0, Trait::AuraBearer);
    Enemy *ally = a.any(EnemyKind::Rat, 170, 10, Trait::Hasted);
    a.run(3);
    assert(ally->frenzy > 0);
  }
  std::cout << "[TEST] Cursed, silencing, enchanted, stone skin, multishot and aura bearer "
               "traits: PASS"
            << std::endl;
}
