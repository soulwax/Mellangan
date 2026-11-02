#undef NDEBUG
#include <cassert>

#include <cmath>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Delve.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Chest.hpp"
#include "level/Level.hpp"
#include "level/Prop.hpp"
#include "level/World.hpp"

namespace {

bool isMine(const Structure *s, const void *) { return s->kind == StructureKind::Mine; }

void clear(Level &level, const Delve &d) {
  level.each<Enemy>([&](Enemy &e) {
    if (std::hypot(e.x - d.x, e.y - d.y) < Delve::RING * 16)
      e.hurt(e.maxHp * 1000, 1, 0, 0);
  });
  for (int i = 0; i < 3; i++)
    level.tick();
}

bool rubble(int tx, int ty) {
  Prop p;
  return world::propAt(tx, ty, p) && p.solid >= 9;
}

} // namespace

void testDelves() {
  assert(Delve::guardian(1) == EnemyKind::Cyclops && Delve::guardian(3) == EnemyKind::Iceback);

  Game game;
  Campaign c;
  c.world = "DELVE001";
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  level.tick();
  Player &p = *level.player;
  p.god = true;

  Structure mine;
  assert(world::nearestStructure(c.xTown, c.yTown, 60, mine, isMine));
  p.x = mine.centerX() + 10, p.y = mine.centerY();
  level.each<Enemy>([](Enemy &e) { e.remove(); });
  level.tick();
  Structure at;
  assert(Delve::entrance(level, c, at) && at.rx == mine.rx && at.ry == mine.ry);
  assert(c.delve.start(level, c, at) && !c.delve.start(level, c, at));
  assert(level.rules.spawnRate == 0);
  int cx = int(std::floor(c.delve.x / 16)), cy = int(std::floor(c.delve.y / 16));
  assert(rubble(cx + Delve::RING, cy) && rubble(cx - Delve::RING, cy) &&
         rubble(cx, cy + Delve::RING) && !rubble(cx, cy));

  for (int f = 1; f <= Delve::FLOORS; f++) {
    for (int i = 0; i < 400 && c.delve.floor < f; i++) {
      if (c.delve.wait <= 0 && c.delve.floor == f - 1)
        clear(level, c.delve);
      level.tick(), c.tick(level);
    }
    level.tick(), level.tick();
    assert(c.delve.floor == f && c.delve.inside(level) > 0);
    assert(p.dim > 0);
    if (f == Delve::FLOORS) {
      bool guardian = false;
      level.each<Enemy>([&](Enemy &e) { guardian |= e.boss && e.is(Delve::guardian(c.act)); });
      assert(guardian);
    }
    clear(level, c.delve);
  }
  for (int i = 0; i < 400 && c.delve.active; i++)
    level.tick(), c.tick(level);
  assert(!c.delve.active && c.delved(mine.rx, mine.ry) && !rubble(cx + Delve::RING, cy));
  level.tick();
  int chests = 0;
  for (auto [id, o] : level.reg.view<Owned>().each())
    chests += dynamic_cast<Chest *>(o.self.get()) != nullptr;
  assert((chests > 0 || p.pendingTreasures > 0) && !Delve::entrance(level, c, at) && level.rules.spawnRate == 1);

  c.delve.start(level, c, mine);
  assert(rubble(cx + Delve::RING, cy));
  c.fall(level);
  assert(!c.delve.active && !rubble(cx + Delve::RING, cy));

  c.hardcore = true;
  c.capture(level);
  Campaign back;
  assert(Campaign::load(c.save(), back) && back.hardcore && back.nightmare == 0 &&
         back.delved(mine.rx, mine.ry));

  float before = level.rules.threat(c.xTown + 800, c.yTown);
  c.stage = QuestStage::Done, c.act = ACT_COUNT, c.quest = QUEST_COUNT;
  c.awaken(level);
  assert(c.nightmare == 1 && c.act == 1 && c.quest == 0 && c.stage == QuestStage::Offered);
  assert(level.rules.threat(c.xTown + 800, c.yTown) > before * 1.9f && c.delves.empty());
  assert(Campaign::load(c.save(), back) && back.nightmare == 1);
  c.leave();
  std::cout << "[TEST] Delves: a mine mouth seals the hero in with rubble, three floors of "
               "underground foes end in the act's guardian boss, the cleared mine pays out and "
               "stays cleared, falling unseals it; hardcore and nightmare save, and nightmare "
               "restarts the acts with stronger foes: PASS"
            << std::endl;
}
