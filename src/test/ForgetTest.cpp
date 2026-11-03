#undef NDEBUG
#include <cassert>

#include <array>
#include <cmath>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "item/Passive.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "item/tree/Tree.hpp"
#include "level/Level.hpp"

void testForget() {
  Game game;
  game.beginCampaign();
  Campaign &c = *game.campaign;
  Player *p = game.level->player;
  game.level->tick();

  Upgrade *relic = nullptr;
  for (auto &u : p->upgrades)
    if (u->rare && !relic)
      relic = u.get();
  assert(relic);
  relic->levelUp(*p);
  auto stats = [](const Player &h) {
    return std::array<float, 8>{h.maxHp, h.might, h.speed, h.haste,
                                h.magnet, h.armor, h.regen, h.luck};
  };
  auto fresh = stats(*p);
  std::string relicName = relic->name;

  p->lvl = 20, c.points = 12, p->gold = 1000;
  Tree tree = Tree::of(*p);
  int spent = 0;
  for (const Node &n : tree.nodes) {
    bool passive = dynamic_cast<Passive *>(n.up) != nullptr;
    for (int k = 0; k < 3 && passive && spent < 6 && tree.spend(*p, n, c.points); k++)
      spent++;
  }
  for (const Node &n : tree.nodes)
    if (n.tab == Tab::Arts && n.tier == 1 && spent < 9)
      while (spent < 9 && tree.spend(*p, n, c.points))
        spent++;
  assert(spent == 9 && c.points == 3);
  assert(stats(*p) != fresh);
  int cost = c.forgetCost();

  assert(game.forget());
  p = game.level->player;
  assert(c.points == 12 && p->gold == 1000 - cost);
  for (size_t i = 0; i < fresh.size(); i++)
    assert(std::abs(stats(*p)[i] - fresh[i]) < 1e-3f);
  assert(p->upgrade(relicName)->level == 1);
  for (auto &u : p->upgrades)
    assert(u->rare || u->level <= 1);
  p->gold = 0;
  assert(!game.forget() && c.points == 12);

  Upgrade *weapon = nullptr;
  for (auto &u : p->upgrades)
    if (u->weapon && u->evolvedName && !u->rare && u->level == 0 && !weapon)
      weapon = u.get();
  assert(weapon);
  std::string base = weapon->name;
  while (weapon->level < Upgrade::MAX_LEVEL)
    weapon->levelUp(*p);
  weapon->evolve();
  std::string evolvedName = weapon->name;
  c.capture(*game.level);
  Campaign back;
  assert(Campaign::load(c.save(), back));
  c.leave();
  Game again;
  again.level = std::make_unique<Level>(again, back.rules());
  back.enter(*again.level);
  Upgrade *same = again.level->player->upgrade(evolvedName);
  assert(same && same->evolved && same->level == Upgrade::MAX_LEVEL && base != evolvedName);
  back.leave();
  std::cout << "[TEST] Forget: the elder's rite refunds every spent point, restores the fresh "
               "hero's stats, keeps relics and costs gold; evolved weapons survive a reload: "
               "PASS"
            << std::endl;
}
