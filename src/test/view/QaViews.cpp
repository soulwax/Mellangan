#undef NDEBUG
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#endif

#include "test/Views.hpp"

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Game.hpp"
#include "level/Chart.hpp"
#include "level/Noise.h"
#include "level/World.hpp"
#include "entity/Player.hpp"
#include "gfx/EnemyWarning.hpp"
#include "menu/LevelUpMenu.hpp"
#include "menu/ShrineMenu.hpp"
#include "level/ExpansionEncounter.hpp"
#include "item/Upgrade.hpp"
#include "entity/enemy/ai/Melee.hpp"
#include "entity/projectile/Orb.hpp"
#include "particle/Spark.hpp"
#include "menu/TitleMenu.hpp"
#include "menu/ClassMenu.hpp"
#include "entity/pickup/Gem.hpp"

void viewDeathEffects(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &player = *level.player;
  player.god = true;
  level.night.dawn = true;
  const EnemyKind kinds[] = {EnemyKind::Rat, EnemyKind::Ghost, EnemyKind::Demon,
                             EnemyKind::Slime, EnemyKind::Bat, EnemyKind::Spider};
  bool running = true;
  uint64_t ends = SDL_GetTicks() + 20000;
  for (int frame = 0; running && SDL_GetTicks() < ends; frame++) {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
    if (frame % 120 == 0)
      for (int i = 0; i < 6; i++) {
        auto enemy = Enemy::create(kinds[i], 1, 1);
        Enemy *target = enemy.get();
        target->x = player.x - 150 + i * 60;
        target->y = player.y;
        target->z = i == 4 ? 7 : 0;
        level.add(std::move(enemy));
        target->hurt(target->hp + 1, 0, 0, 0);
      }
    level.tick();
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(player.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(player.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.35f, 0.35f, 0.35f, 1});
    level.renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level.render(game.screen);
    enemyWarning(game.screen, player.x, player.y + 58, 24, 0.9f, 0);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    const char *labels[] = {"RAT", "GHOST", "DEMON", "SLIME", "BAT", "SPIDER"};
    for (int i = 0; i < 6; i++)
      game.screen.textCenter(labels[i], 90.0f + i * 60, 95, 0.8f, col::white);
    game.screen.textCenter("ENEMY WARNING", 240, 205, 0.8f, col::red);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}

void viewGui(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  LevelUpMenu upgrades(game);
  ShrineMenu shrine(game);
  upgrades.t = shrine.t = 1;
  uint64_t start = SDL_GetTicks();
  bool running = true;
  while (running && SDL_GetTicks() - start < 20000) {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
    Player &p = *game.level->player;
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(p.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(p.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.35f, 0.35f, 0.35f, 1});
    game.level->renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    game.level->render(game.screen);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    if ((SDL_GetTicks() - start) / 5000 % 2 == 0)
      upgrades.render(game.screen);
    else
      shrine.render(game.screen);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}

void viewBiomeEncounter(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &upgrade : p.upgrades)
    upgrade->level = 0;
  level.night.dawn = true;
  level.time = 120;
  int tx = 0, ty = 0;
  bool found = false;
  for (int y = -300; y <= 300 && !found; y += 20)
    for (int x = -300; x <= 300 && !found; x += 20)
      if (world::biomeAt(x, y) == BIOME_FOREST)
        tx = x, ty = y, found = true;
  p.x = tx * 16, p.y = ty * 16;
  Structure site{StructureKind::Ruin, tx - 1, ty - 1, 2, 2, 0, 0, 0};
  level.add<ExpansionChallenge>(site, BIOME_FOREST);
  bool running = true;
  uint64_t end = SDL_GetTicks() + 20000;
  while (running && SDL_GetTicks() < end) {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
    level.tick();
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(p.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(p.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.35f, 0.35f, 0.35f, 1});
    level.renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level.render(game.screen);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    game.screen.textCenter("FOREST CHALLENGE: DEFEAT THE GUARDS", game.screen.w / 2,
                           16 * game.screen.u, game.screen.u, col::gold);
    game.screen.textCenter(std::string("REWARD: ") + biomeRewardText(BIOME_FOREST),
                           game.screen.w / 2, 34 * game.screen.u, game.screen.u, col::white);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}

void viewCombatReadability(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &upgrade : p.upgrades)
    upgrade->level = 0;
  level.night.dawn = true;
  for (int i = 0; i < int(StatusKind::Count); i++)
    p.afflict(StatusKind(i), 60, 0.1f);
  Melee windup(2, 0.4f, 1, 1);
  auto enemy = Enemy::create(EnemyKind::Rat, 10, 1);
  Enemy *target = enemy.get();
  target->x = p.x + 60;
  target->y = p.y;
  target->plan = nullptr;
  target->goals.clear();
  target->goals.push(&windup);
  target->goalTime = 1;
  target->xMark = -1;
  target->yMark = 0;
  for (int i = 0; i < int(StatusKind::Count); i++)
    target->afflict(StatusKind(i), 60, 0.1f);
  level.add(std::move(enemy));
  bool running = true;
  uint64_t end = SDL_GetTicks() + 20000;
  int frame = 0;
  while (running && SDL_GetTicks() < end) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
      if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_V)
        game.settings.reduced = !game.settings.reduced;
    }
    if (frame % 180 == 0)
      level.add<Orb>(p.x - 50, p.y - 12, 0, 0, 0);
    if (frame % 20 == 0)
      Spark::burst(level, p.x, p.y - 15, col::gold, 14, 100, true);
    level.tick();
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(p.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(p.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.3f, 0.3f, 0.3f, 1});
    level.renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level.render(game.screen);
    enemyWarning(game.screen, p.x + 110, p.y, 18, 0.9f, 0);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    game.screen.textCenter(game.settings.reduced ? "V: REDUCED PARTICLES" : "V: FULL PARTICLES",
                           game.screen.w / 2, 12 * game.screen.u, game.screen.u, col::white);
    vk2dRendererPresent();
    SDL_Delay(16);
    frame++;
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}
