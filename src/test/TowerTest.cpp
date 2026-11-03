#undef NDEBUG
#include <cassert>

#include <cmath>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Watchtower.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

void testWatchtower() {
  Game game;
  Campaign c;
  c.world = "TOWERS01";
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  level.tick();
  Player &p = *level.player;
  p.god = true;
  level.each<Enemy>([](Enemy &e) { e.remove(); });

  assert(!c.fortify(level).empty() && c.towers.empty());
  p.gold = 400, p.materials[int(Mat::Stone)] = 12, p.materials[int(Mat::Wood)] = 8;
  assert(c.fortify(level).empty() && c.fortified(0) && p.gold == 100);
  assert(p.materials[int(Mat::Stone)] == 0 && p.materials[int(Mat::Wood)] == 0);
  p.gold = 1000, p.materials[int(Mat::Stone)] = 99, p.materials[int(Mat::Wood)] = 99;
  assert(!c.fortify(level).empty() && c.towers.size() == 1);
  level.tick();
  Watchtower *tower = level.any<Watchtower>();
  assert(tower);

  auto made = Enemy::create(EnemyKind::Rat, 50, 1);
  Enemy *rat = made.get();
  rat->x = tower->x + 120, rat->y = tower->y - 30;
  level.add(std::move(made));
  level.tick();
  float hp = rat->hp;
  for (int i = 0; i < 180 && rat->hp == hp; i++) {
    rat->x = tower->x + 120, rat->y = tower->y - 30;
    level.tick();
  }
  assert(tower->shots > 0 && rat->hp < hp);

  c.capture(level);
  Campaign back;
  assert(Campaign::load(c.save(), back) && back.fortified(0));
  c.leave();
  Game again;
  again.level = std::make_unique<Level>(again, back.rules());
  back.enter(*again.level);
  again.level->tick();
  assert(again.level->any<Watchtower>());
  back.leave();
  std::cout << "[TEST] Watchtower: the elder raises one per town for gold, stone and wood, it "
               "shoots foes near the fence, and it stands again after a reload: PASS"
            << std::endl;
}
