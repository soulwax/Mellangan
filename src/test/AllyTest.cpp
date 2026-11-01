#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Ally.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/Level.hpp"

namespace {

struct Field {
  Game game;
  Level &level;
  Player &p;

  Field() : level(*(game.level = std::make_unique<Level>(game, quiet()))), p(*level.player) {
    for (auto &up : p.upgrades)
      up->level = 0;
    p.maxHp = p.hp = 1e5f;
  }
  ~Field() { game.level.reset(); }

  static RunRules quiet() {
    RunRules r;
    r.spawnRate = 0, r.waves = false, r.bosses = false;
    return r;
  }

  Enemy *foe(EnemyKind kind, double dx, double dy) {
    auto made = Enemy::create(kind, 1, 1);
    Enemy *e = made.get();
    e->x = p.x + dx, e->y = p.y + dy;
    level.add(std::move(made));
    return e;
  }

  void run(int ticks) {
    for (int i = 0; i < ticks; i++)
      level.tick();
  }
};

Recipe totem(int count, unsigned payload = 0) {
  return {"Wolf Totem", nullptr, "", "wolf", 0, Form::Summon, payload, 10, 3, 0, 0, count,
          col::white, atlas::pt::Rune};
}

} // namespace

void testAllies() {
  {
    Field f;
    Recipe r = totem(3);
    fireRecipe(r, f.p, nullptr, 10, 0, 3);
    f.run(1);
    assert(f.level.allies.size() == 3);
    fireRecipe(r, f.p, nullptr, 10, 0, 3);
    f.run(1);
    assert(f.level.allies.size() == 3);
    for (Ally *a : f.level.allies)
      assert(&a->def == &allyDef(AllyKind::Wolf) && !a->hostile());

    std::vector<Mob *> near = f.level.nearestFoes(true, f.p.x, f.p.y, 100, 8);
    assert(near.size() == 4);

    Enemy *rat = f.foe(EnemyKind::Rat, 60, 0);
    entt::entity ratId = rat->id;
    float ratHp = rat->hp;
    f.p.god = true;
    f.run(180);
    rat = f.level.get<Enemy>(ratId);
    assert(!rat || rat->hp < ratHp);

    Recipe ring{"Ring", nullptr, "", "ring", 0, Form::Nova, pay::Burn | pay::Explode, 50, 1, 90,
                0, 1, col::white, atlas::pt::Ember};
    std::vector<float> before;
    for (Ally *a : f.level.allies)
      before.push_back(a->hp);
    fireRecipe(ring, f.p, nullptr, 50, 90, 1);
    f.run(30);
    for (size_t i = 0; i < f.level.allies.size() && i < before.size(); i++)
      assert(f.level.allies[i]->hp >= before[i]);

    f.run(int(allyDef(AllyKind::Wolf).life / DT) + 10);
    assert(f.level.allies.empty() && f.level.count<Ally>() == 0);
    f.run(5);
  }
  {
    Field f;
    Ally *wolf = summon(f.level, AllyKind::Wolf, f.p.x + 90, f.p.y, 10, 30);
    f.run(1);
    Enemy *knight = f.foe(EnemyKind::Knight, 150, 0);
    f.run(1);
    assert(&knight->quarry() == wolf);
    entt::entity wolfId = wolf->id;
    float hp = wolf->hp;
    for (int i = 0; i < 240 && wolf && wolf->hp >= hp; i++) {
      wolf->life = 30;
      f.run(1);
      wolf = f.level.get<Ally>(wolfId);
    }
    assert(!wolf || wolf->hp < hp);
  }
  {
    Field f;
    Ally *decoy = summon(f.level, AllyKind::Decoy, f.p.x + 60, f.p.y, 0, 4);
    Enemy *bat = f.foe(EnemyKind::Bat, 40, 40);
    f.run(1);
    assert(&bat->quarry() == decoy);
    f.run(int(4 / DT) + 5);
    assert(f.level.allies.empty());
    assert(&bat->quarry() == &f.p);
  }
  std::cout << "[TEST] Allies summon to their cap, bite foes, take no friendly "
               "fire, draw foes, decoys lure, and retire cleanly: PASS"
            << std::endl;
}
