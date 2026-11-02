#undef NDEBUG
#include <cassert>

#include <cmath>
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
  assert(c.stage == QuestStage::Road && c.act == 2 && c.quest == 4);

  for (int act = 2; act <= ACT_COUNT; act++) {
    double rx, ry;
    assert(quests::road(act, rx, ry));
    for (const Campaign::Town &t : c.towns())
      assert(std::hypot(t.x - rx, t.y - ry) > 600);
    p.x = rx + 500, p.y = ry;
    quests::tick(level, c);
    assert(c.stage == QuestStage::Road);
    p.x = rx + 40, p.y = ry;
    quests::tick(level, c);
    level.tick();
    assert(c.stage == QuestStage::Offered && int(c.towns().size()) == act);
    assert(c.home().x == rx && level.rules.safeAt(rx, ry));
    Elder *elder = level.get<Elder>(c.elder);
    assert(elder && std::hypot(elder->x - rx, elder->y - ry) < 60);
    for (int k = 0; k < QUESTS_PER_ACT; k++) {
      const QuestDef &q = *quests::current(c);
      quests::accept(level, c);
      assert(c.stage == QuestStage::Active);
      if (q.goal == Objective::Kindle)
        level.night.lit += q.count;
      else if (q.goal == Objective::Hunt)
        level.slain[int(q.prey[0])] += q.count;
      else {
        go(level, c);
        Enemy *e = level.get<Enemy>(c.foe);
        assert(e && (q.goal != Objective::Boss || (e->boss && e->is(q.prey[0]))));
        slay(level, c);
      }
      quests::tick(level, c);
      assert(c.stage == QuestStage::Ready);
      quests::turnIn(level, c);
    }
  }
  assert(c.stage == QuestStage::Done && c.act == ACT_COUNT && !quests::current(c));
  assert(quests::status(c) == "The dark is broken");
  c.capture(level);
  Campaign saved;
  assert(Campaign::load(c.save(), saved) && saved.outposts.size() == ACT_COUNT - 1 &&
         saved.stage == QuestStage::Done);
  for (size_t i = 0; i < saved.outposts.size(); i++)
    assert(std::abs(saved.outposts[i].x - c.outposts[i].x) < 0.1);
  c.travel(level, 2);
  assert(std::hypot(p.x - c.towns()[2].x, p.y - c.towns()[2].y) < 30);
  c.leave();
  std::cout << "[TEST] Quests: the elder stands in town; Act I runs kindle, slay, hunt and the "
               "Pack Lord; each act opens a road to a new barricaded town with its elder, four "
               "quests and its boss, through the Cinder King; towns save and waypoints travel: "
               "PASS"
            << std::endl;
}
