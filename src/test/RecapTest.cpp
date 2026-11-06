#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <cstring>
#include <iostream>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "menu/Recap.hpp"

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
