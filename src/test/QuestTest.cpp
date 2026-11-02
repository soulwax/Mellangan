#undef NDEBUG
#include <cassert>

#include <cstring>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Elder.hpp"
#include "campaign/Quests.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

namespace {

void go(Level &level, Campaign &c) {
  Player &p = *level.player;
  p.x = c.qx, p.y = c.qy;
  quests::tick(level, c);
  level.tick();
}

void slay(Level &level, Campaign &c) {
  Enemy *foe = level.get<Enemy>(c.foe);
  assert(foe);
  foe->hurt(foe->maxHp * 1000, 1, 0, 0);
  for (int i = 0; i < 3; i++)
    level.tick();
  quests::tick(level, c);
}

} // namespace

void testQuests() {
  Game game;
  Campaign c;
  c.world = "QUESTS01";
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  level.tick();
  Player &p = *level.player;
  p.god = true;
  assert(c.quest == 0 && c.stage == QuestStage::Offered && level.get<Elder>(c.elder));

  quests::accept(level, c);
  assert(c.stage == QuestStage::Active && !c.qsite);
  level.night.lit = 2;
  quests::tick(level, c);
  assert(c.progress == 2 && c.stage == QuestStage::Active);
  level.night.lit = 3;
  quests::tick(level, c);
  assert(c.stage == QuestStage::Ready);
  int gold = p.gold;
  quests::turnIn(level, c);
  assert(p.gold == gold + QUESTS[0].gold && c.quest == 1 && c.stage == QuestStage::Offered);

  quests::accept(level, c);
  assert(c.qsite && c.foe == entt::null);
  quests::tick(level, c);
  assert(c.foe == entt::null);
  go(level, c);
  Enemy *keeper = level.get<Enemy>(c.foe);
  assert(keeper && keeper->title && std::strcmp(keeper->title, "The Pale Warden") == 0);
  slay(level, c);
  assert(c.stage == QuestStage::Ready);
  quests::turnIn(level, c);

  quests::accept(level, c);
  level.slain[int(EnemyKind::Wolf)] += 9;
  quests::tick(level, c);
  assert(c.progress == 9);
  c.capture(level);
  Campaign back;
  assert(Campaign::load(c.save(), back));
  assert(back.quest == 2 && back.stage == QuestStage::Active && back.progress == 9 &&
         back.act == 1);
  level.slain[int(EnemyKind::Alpha)] += 6, level.slain[int(EnemyKind::MoonHound)] += 5;
  quests::tick(level, c);
  assert(c.stage == QuestStage::Ready && c.progress == 20);
  quests::turnIn(level, c);

  quests::accept(level, c);
  go(level, c);
  Enemy *lord = level.get<Enemy>(c.foe);
  assert(lord && lord->boss && lord->is(EnemyKind::Alpha));
  slay(level, c);
  assert(c.stage == QuestStage::Ready);
  quests::turnIn(level, c);
  assert(c.stage == QuestStage::Done && c.act == 2 && !quests::current(c));
  assert(quests::status(c) == "Act I cleared");
  c.leave();
  std::cout << "[TEST] Quests: the elder stands in town; Act I runs kindle, slay the Pale Warden "
               "at the barrow, hunt twenty of the pack and slay the Pack Lord at his den; "
               "rewards pay out, progress survives a save, and the act clears: PASS"
            << std::endl;
}
