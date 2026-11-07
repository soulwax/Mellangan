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
#include "menu/ClassMenu.hpp"
#include "menu/UpgradeCard.hpp"
#include "menu/Recap.hpp"
#include "meta/Feats.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Quests.hpp"
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
  assert(unlockFeats(game, false, true).empty() && !has("camp_act1"));
  game.campaign = std::make_unique<Campaign>();
  game.campaign->quest = QUESTS_PER_ACT, game.campaign->hardcore = true;
  l.player->lvl = 40;
  fresh = unlockFeats(game, false, true);
  assert(fresh.size() == 2 && has("camp_act1") && has("camp_hardcore") && !has("camp_act2") && !has("level_40"));
  game.campaign->quest = QUEST_COUNT, game.campaign->nightmare = 1, game.campaign->rescued = 5;
  game.campaign->towers = {0}, game.campaign->delves = {{1, 1}, {2, 2}, {3, 3}};
  unlockFeats(game, false, true);
  for (const char *id : {"camp_act2", "camp_act3", "camp_act4", "camp_act5", "camp_nightmare", "camp_tower",
                         "camp_shelter", "camp_delve"})
    assert(has(id));
  assert(!has("level_40") && unlockFeats(game, false, true).empty());
  game.campaign.reset();
  game.level.reset();
  rng = kept;
  std::cout << "[TEST] Achievements: " << FEAT_COUNT
            << " with unique ids, each unlocks once when its moment comes, dawn with all twelve "
               "classes follows the twelve, and campaign milestones unlock without run feats: PASS"
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

void testEndless() {
  Rng kept = rng;
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &l = *game.level;
  l.time = 900, l.player->god = true;
  int runs = game.codex.runs;
  game.beginEndless();
  assert(game.endless && !l.night.dawn && !l.player->god && l.night.left() > 1e8f);
  float hp = game.balance.enemyHp;
  l.time = game.endlessFrom + 60;
  game.observe();
  assert(std::abs(game.balance.enemyHp - hp * 1.1f) < 1e-4f);
  l.time = game.endlessFrom + 75;
  game.onPlayerDied();
  assert(std::abs(game.codex.bestEndless - 75) < 0.01f && game.codex.runs == runs);
  game.newGame();
  assert(!game.endless);
  game.level.reset();
  rng = kept;
  std::cout << "[TEST] Endless night: dawn gives way to a night that never ends, it deepens every "
               "minute, and a fall keeps the record without counting a second run: PASS"
            << std::endl;
}

void testDaily() {
  Rng kept = rng;
  auto roll = [](const char *code, bool push) {
    Game game;
    game.level = std::make_unique<Level>(game);
    game.worldMode = WorldMode::Daily, game.seedCode = code;
    ClassMenu menu(game);
    menu.tick();
    if (push) {
      game.input.right.toggle(true), game.input.up.toggle(true);
      game.input.tick();
      menu.tick();
    }
    std::pair<int, int> out{game.level->player->tile, game.heat};
    game.level.reset();
    return out;
  };
  auto a = roll("D20261106", false), b = roll("D20261106", true);
  assert(a == b && a.second >= 0 && a.second <= 3);
  bool varies = false;
  for (const char *day : {"D20261107", "D20261108", "D20261109", "D20261110", "D20261111"}) {
    auto d = roll(day, false);
    assert(d.second >= 0 && d.second <= 3);
    varies |= d != a;
  }
  assert(varies);
  rng = kept;
  std::cout << "[TEST] Daily: the date fixes the class and a heat of 0-3 for everyone, the arrows "
               "cannot change them, and different days differ: PASS"
            << std::endl;
}

void testFigures() {
  int weapons = 0;
  std::string silent;
  for (auto &u : Upgrade::all()) {
    if (!u->weapon || u->rare || u->skill)
      continue;
    weapons++;
    Figures one = u->figures(1, false);
    assert(one.n >= 2);
    for (int l = 1; l < Upgrade::MAX_LEVEL; l++) {
      Figures a = u->figures(l, false), b = u->figures(l + 1, false);
      assert(a.n == b.n);
      bool moved = false;
      for (int i = 0; i < a.n; i++)
        moved |= std::strcmp(a.line[i].name, b.line[i].name) != 0 || a.line[i].value != b.line[i].value;
      if (!moved)
        silent += fmt::format(" {} L{}", u->name, l + 1);
    }
  }
  if (!silent.empty())
    std::cout << "[TEST] Levels that change no figure:" << silent << std::endl;
  assert(silent.empty());
  Rng kept = rng;
  Game game;
  game.level = std::make_unique<Level>(game);
  Player &p = *game.level->player;
  Upgrade *staff = p.upgrade("Arcane Staff");
  assert(staff);
  staff->level = 1;
  p.might = 1.5f;
  std::vector<std::string> up = UpgradeCard(staff, p, false, 0).numbers();
  assert(!up.empty() && up[0] == "damage 15 > 27");
  staff->level = 0;
  assert(UpgradeCard(staff, p, false, 0).numbers()[0] == "damage 15");
  staff->level = Upgrade::MAX_LEVEL;
  assert(UpgradeCard(staff, p, true, 0).numbers()[0] == "damage 39 > 78");
  game.level.reset();
  rng = kept;
  std::cout << "[TEST] Figures: all " << weapons
            << " weapons report their numbers per level, every level changes at least one, and "
               "cards show might-scaled before and after values, new picks and evolutions: PASS"
            << std::endl;
}

void testSynergy() {
  Rng kept = rng;
  Game game;
  game.level = std::make_unique<Level>(game);
  Player &p = *game.level->player;
  auto staff = Upgrade::all();
  const Upgrade *weapon = nullptr, *tonic = nullptr;
  for (auto &u : staff)
    if (std::strcmp(u->name, "Arcane Staff") == 0)
      weapon = u.get();
    else if (std::strcmp(u->name, "Hourglass Tonic") == 0)
      tonic = u.get();
  assert(weapon && tonic);
  if (Upgrade *held = p.upgrade("Hourglass Tonic"))
    held->level = 0;
  bool ready = true;
  assert(UpgradeCard(weapon, p, false, 0).synergy(ready) == "evolves with Hourglass Tonic" && !ready);
  if (Upgrade *held = p.upgrade("Arcane Staff"))
    held->level = 0;
  assert(UpgradeCard(tonic, p, false, 0).synergy(ready).empty());
  p.grant("Arcane Staff", 1);
  assert(UpgradeCard(tonic, p, false, 0).synergy(ready) == "evolves your Arcane Staff" && ready);
  p.grant("Hourglass Tonic", 1);
  UpgradeCard(weapon, p, false, 0).synergy(ready);
  assert(ready);
  int lonely = 0;
  for (auto &u : staff) {
    if (!u->weapon || u->skill || u->rare)
      continue;
    bool pairs = false;
    for (const auto &e : evolutionTable())
      pairs |= std::strcmp(e[0], u->name) == 0;
    lonely += !pairs && !u->evolvedName;
  }
  assert(lonely == 0);
  game.level.reset();
  rng = kept;
  std::cout << "[TEST] Synergy: weapon cards name their evolution partner and say when you hold it, "
               "partner cards name the weapon they would evolve, and every weapon has an evolution: PASS"
            << std::endl;
}
