#undef NDEBUG
#include <cassert>

#include <cmath>
#include <cstring>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Elder.hpp"
#include "campaign/Pilgrim.hpp"
#include "campaign/Quests.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/WildFoe.hpp"
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
      else if (q.goal == Objective::Gather) {
        p.materials[q.material] = 0;
        quests::tick(level, c);
        assert(c.stage == QuestStage::Active && c.progress == 0);
        p.materials[q.material] = q.count + 2;
      } else if (q.goal == Objective::Escort) {
        quests::tick(level, c);
        level.tick();
        Pilgrim *w = level.get<Pilgrim>(c.pilgrim);
        assert(w && c.qsite);
        w->hp = -1;
        level.tick();
        quests::tick(level, c);
        assert(c.stage == QuestStage::Offered && c.progress == 0);
        quests::accept(level, c);
        quests::tick(level, c);
        level.tick();
        w = level.get<Pilgrim>(c.pilgrim);
        assert(w);
        w->hp = 1e9f;
        double sx = w->x, sy = w->y;
        p.x = w->x + 400, p.y = w->y;
        for (int i = 0; i < 60; i++)
          level.tick(), quests::tick(level, c);
        assert(w->waiting && std::hypot(w->x - sx, w->y - sy) < 1);
        int moved = 0;
        for (int i = 0; i < 60 * 120 && c.stage == QuestStage::Active; i++) {
          p.x = w->x + 30, p.y = w->y;
          level.tick(), quests::tick(level, c);
          moved = std::max(moved, c.progress);
        }
        assert(moved > 0);
      } else if (q.goal == Objective::Purge) {
        go(level, c);
        assert(int(c.nests.size()) == q.count && c.progress == 0 && c.stage == QuestStage::Active);
        auto *nest = dynamic_cast<WildFoe *>(level.get<Enemy>(c.nests[0]));
        assert(nest && nest->is(EnemyKind::Nest) && nest->brood == q.prey[0]);
        Enemy *last = level.get<Enemy>(c.nests.back());
        last->hp = last->maxHp = 1e12f;
        int brood = 0;
        for (int i = 0; i < 400 && !brood; i++) {
          p.x = c.qx + 260, p.y = c.qy;
          level.tick();
          level.each<Enemy>([&](Enemy &e) { brood += e.is(q.prey[0]); });
        }
        assert(brood > 0);
        for (size_t n = 0; n + 1 < c.nests.size(); n++)
          if (Enemy *e = level.get<Enemy>(c.nests[n]))
            e->hurt(1e9f, 1, 0, 0);
        level.tick(), quests::tick(level, c);
        assert(c.progress == q.count - 1 && c.stage == QuestStage::Active);
        level.get<Enemy>(c.nests.back())->hurt(1e15f, 1, 0, 0);
        level.tick();
      } else if (q.goal == Objective::Defend) {
        go(level, c);
        assert(c.holding);
        p.x = c.qx + 300, p.y = c.qy;
        for (int i = 0; i < 180; i++)
          level.tick(), quests::tick(level, c);
        assert(c.progress == 0);
        p.x = c.qx, p.y = c.qy;
        int waves = 0;
        for (int i = 0; i < 60 * (q.count + 2) && c.stage == QuestStage::Active; i++) {
          level.tick(), quests::tick(level, c);
          waves += c.waveTime > quests::WAVE - DT * 1.5f;
          p.x = c.qx, p.y = c.qy, p.hp = p.maxHp;
        }
        assert(waves >= q.count / int(quests::WAVE) - 1);
      } else {
        go(level, c);
        Enemy *e = level.get<Enemy>(c.foe);
        assert(e && (q.goal != Objective::Boss || (e->boss && e->is(q.prey[0]))));
        slay(level, c);
      }
      quests::tick(level, c);
      assert(c.stage == QuestStage::Ready);
      quests::turnIn(level, c);
      if (q.goal == Objective::Gather)
        assert(p.materials[q.material] == 2);
    }
  }
  assert(c.stage == QuestStage::Done && c.act == ACT_COUNT && !quests::current(c));
  assert(quests::status(c) == "The dark is broken");
  c.capture(level);
  for (size_t i = 0; i < c.towns().size(); i++) {
    int tx = int(std::floor(c.towns()[i].x / 16)) - 1, ty = int(std::floor(c.towns()[i].y / 16)) - 7;
    constexpr Piece HEDGES[] = {Piece::Thorns, Piece::Reeds, Piece::Ice, Piece::Cactus, Piece::Embers};
    assert(c.edits.at(tx, ty) && c.edits.at(tx, ty)->piece == HEDGES[i]);
  }
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
