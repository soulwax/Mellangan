#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "menu/Recap.hpp"
#include "meta/Feats.hpp"
#include "meta/Heat.hpp"
#include "level/RunRules.hpp"
#include <cmath>
#include "meta/RunLog.hpp"
#include "hero/Hero.hpp"

void testAdvice() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &l = *game.level;
  Player &p = *l.player;
  auto says = [&](const char *word) { return std::strstr(advice(l), word) != nullptr; };

  l.time = 30;
  assert(says("first second"));
  l.time = 400, l.distanceRan = 400 * 60.0f, p.lvl = 20;
  assert(l.night.lit == 0 && says("shrines"));
  l.night.lit = 1;
  for (auto &up : p.upgrades)
    up->level = 0;
  for (const char *name : {"Arcane Staff", "Frost Nova", "Meteor"})
    if (Upgrade *up = p.grant(name, 1))
      up->level = 2;
  assert(says("level 5"));
  for (auto &up : p.upgrades)
    if (up->weapon && up->level > 0)
      up->level = Upgrade::MAX_LEVEL;
  assert(!says("level 5") && std::strlen(advice(l)) > 0);
  l.night.lit = 0;
  game.level.reset();
  std::cout << "[TEST] Death advice: early deaths, unlit shrines and weapons spread thin each get "
               "their own tip, and there is always one: PASS"
            << std::endl;
}

void testHints() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &l = *game.level;
  auto run = [&](float seconds) {
    for (int i = 0; i < int(seconds * 60); i++)
      l.time += DT, game.hud.tick(true);
  };
  run(4);
  assert(std::strstr(game.hud.lesson().c_str(), "move") && game.codex.hints == 1);
  run(11);
  assert(std::strstr(game.hud.lesson().c_str(), "gems") && game.codex.hints == 3);
  uint32_t seen = game.codex.hints;
  game.level->player->lvl = 2;
  run(12);
  assert(std::strstr(game.hud.lesson().c_str(), "upgrade") && game.codex.hints == (seen | 4));
  Game again;
  again.level = std::make_unique<Level>(again);
  again.codex.hints = ~0u;
  for (int i = 0; i < 6000; i++)
    again.level->time += DT, again.hud.tick(true);
  assert(again.hud.lesson().empty());
  game.level.reset(), again.level.reset();
  std::cout << "[TEST] Hints: lessons come one at a time in order when their moment arrives, each is "
               "remembered in the codex, and a codex that knows them all shows none: PASS"
            << std::endl;
}

void testRunLog() {
  Rng kept = rng;
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &l = *game.level;
  Player &p = *l.player;
  l.time = 312, l.trace.levels = {1, 4, 7, 9, 11};
  l.trace.biomes[0] = 200, l.trace.biomes[3] = 112;
  l.trace.picks = {"Frost Nova", "Arcane Staff"};
  p.felledBy = "a cyclops, angry";
  for (auto &up : p.upgrades)
    if (up->weapon && up->level > 0)
      up->dealt = 1234;
  std::string row = runRow(game, false);
  auto columns = [](const std::string &s) { return std::count(s.begin(), s.end(), ',') + 1; };
  assert(row.back() == '\n' && columns(row) == columns(RUN_HEADER));
  assert(row.find(",312,0,a cyclops  angry,") != std::string::npos);
  assert(row.find("Frost Nova|Arcane Staff") != std::string::npos);
  assert(row.find(",1|4|7|9|11,") != std::string::npos && row.find(":1234") != std::string::npos);
  assert(row.find("THE MEADOWS:200") != std::string::npos);
  game.level.reset();
  rng = kept;
  std::cout << "[TEST] Run log: one CSV row per run with the header's columns, commas in names "
               "neutralised, weapons, picks, level curve and biome time recorded: PASS"
            << std::endl;
}

void testFeats() {
  Rng kept = rng;
  assert(FEAT_COUNT >= 40 && FEAT_COUNT <= 60);
  for (int i = 0; i < FEAT_COUNT; i++)
    for (int j = i + 1; j < FEAT_COUNT; j++)
      assert(std::strcmp(FEATS[i].id, FEATS[j].id) != 0);
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &l = *game.level;
  l.player->hero = HEROES[0];
  auto has = [&](const char *id) {
    for (int i = 0; i < FEAT_COUNT; i++)
      if (std::strcmp(FEATS[i].id, id) == 0)
        return bool(game.codex.feats >> i & 1);
    assert(false);
    return false;
  };
  assert(unlockFeats(game, false).empty() || !has("first_dawn"));
  game.codex.dawns = 1, l.time = 900;
  std::vector<int> fresh = unlockFeats(game, true);
  assert(!fresh.empty() && has("first_dawn") && has("dawn_mage") && has("dawn_unlit") &&
         !has("dawn_swift") && !has("dawn_all"));
  assert(unlockFeats(game, true).empty());
  for (int h = 1; h < HERO_COUNT; h++) {
    l.player->hero = HEROES[h];
    unlockFeats(game, true);
  }
  assert(has("dawn_all") && has("dawn_corsair"));
  game.level.reset();
  rng = kept;
  std::cout << "[TEST] Achievements: " << FEAT_COUNT
            << " with unique ids, each unlocks once when its moment comes, and dawn with all twelve "
               "classes follows the twelve: PASS"
            << std::endl;
}

void testHeat() {
  Balance base, b;
  RunRules rBase, r;
  applyHeat(0, b, r);
  assert(b.enemyHp == base.enemyHp && r.dawnPull == rBase.dawnPull);
  applyHeat(3, b, r);
  auto near = [](float a, float x) { return std::abs(a - x) < 1e-4f; };
  assert(near(b.enemyHp, base.enemyHp * 1.25f) && near(b.enemySpeed, base.enemySpeed * 1.1f) &&
         near(b.enemyDamage, base.enemyDamage * 1.25f) && b.eliteBase == base.eliteBase);
  Balance all;
  RunRules rAll;
  applyHeat(HEAT_MAX, all, rAll);
  assert(near(rAll.dawnPull, rBase.dawnPull * 0.5f) && near(rAll.nightLength, rBase.nightLength * 1.2f) &&
         near(all.playerHp, base.playerHp * 0.8f) && heatNames(2) == "THICK HIDES   HUNGRY DARK");
  Game game;
  game.loadBalance();
  float hp = game.balance.enemyHp;
  game.heat = 1;
  game.loadBalance();
  assert(near(game.balance.enemyHp, hp * 1.25f));
  game.heat = 0;
  Codex old;
  assert(old.parse("1 0 3 2 0 0 50.0\n1 2\n0\n4\n") && old.heat == 1);
  Codex fresh;
  assert(fresh.parse("1 0 3 0 0 0 50.0\n1 2\n0\n4\n") && fresh.heat == 0);
  std::cout << "[TEST] Heat: each level adds its curse on top of the difficulty tier, heat 0 changes "
               "nothing, and a codex with dawns but no heat line unlocks heat 1: PASS"
            << std::endl;
}
