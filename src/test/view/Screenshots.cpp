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

void captureScreenshotSuite(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.init(window, base))
    return;

  VK2DTexture cap = vk2dTextureCreate(1440, 810);
  if (!cap) {
    game.shutdown();
    return;
  }

  auto snap = [&](const char *path) {
    game.screen.targetGui(cap);
    game.render();
    game.screen.targetGui(nullptr);
    vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
    vk2dRendererPresent();
    vk2dRendererWait();
    game.screen.saveTarget(cap, path);
  };

  game.worldMode = WorldMode::Random;
  game.seedCode = "KXXCCTRM";
  game.reseed();
  for (int i = 0; i < 40; i++) {
    game.tick();
    game.render();
    vk2dRendererPresent();
  }
  snap("static/screenshots/01-title.png");

  game.codex.start = 0;
  game.setMenu(std::make_unique<ClassMenu>(game));
  for (int i = 0; i < 40; i++) {
    game.tick();
    game.render();
    vk2dRendererPresent();
  }
  snap("static/screenshots/02-classes.png");

  game.menu.reset();
  game.dev = true;
  game.biomePick = BIOME_SAVANNA;
  game.visitBiome();
  Player &p = *game.level->player;
  p.hero = HEROES[0];
  HEROES[0]->equip(p);
  p.god = true;
  game.level->time = 100.0f;
  game.level->kills = 164;
  game.level->streak = 150;
  game.level->streakTime = 2.0f;
  p.lvl = 9;
  p.exp = 280;
  p.expNext = 360;
  p.hp = 100;
  p.maxHp = 100;
  game.hud.banner = "THE SAVANNA";
  game.hud.bannerTime = 2.0f;

  for (auto &up : p.upgrades) {
    std::string_view n = up->name;
    if (n == "Staff" || n == "Frost Nova" || n == "Meteor" ||
        n == "Fire Bomb" || n == "Transmute" || n == "Boots" ||
        n == "Amulet" || n == "Tome")
      up->level = std::max(1, up->level);
  }

  for (int i = 0; i < 8; i++) {
    float a = float(i) * 6.28318f / 8.0f;
    float dist = 70.0f + (i % 3) * 20.0f;
    EnemyKind k = (i % 4 == 0) ? EnemyKind::Wolf : (i % 4 == 1) ? EnemyKind::Ghost : (i % 4 == 2) ? EnemyKind::Rat : EnemyKind::Slime;
    auto e = Enemy::create(k, 10, 1);
    e->x = p.x + std::cos(a) * dist;
    e->y = p.y + std::sin(a) * dist;
    game.level->add(std::move(e));
  }

  for (int i = 0; i < 3; i++) {
    float a = float(i) * 6.28318f / 3.0f;
    game.level->add<Orb>(float(p.x + std::cos(a) * 35.0), float(p.y + std::sin(a) * 35.0), 0, 0, 0);
  }
  for (int i = 0; i < 6; i++) {
    float gx = float(p.x + (i - 3) * 18.0 + 10.0);
    float gy = float(p.y + ((i * 7) % 5 - 2) * 15.0);
    game.level->add<Gem>(gx, gy, 5);
  }
  Spark::burst(*game.level, float(p.x + 25), float(p.y - 10), col::gold, 16, 80, true);
  Spark::burst(*game.level, float(p.x - 30), float(p.y + 15), col::xp, 16, 80, true);

  for (int i = 0; i < 8; i++) {
    game.level->tick();
    game.render();
    vk2dRendererPresent();
  }

  game.screen.targetGui(cap);
  game.render();
  enemyWarning(game.screen, float(p.x + 55), float(p.y - 20), 22.0f, 0.9f, 0.0f);
  game.screen.targetGui(nullptr);
  vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
  vk2dRendererPresent();
  vk2dRendererWait();
  game.screen.saveTarget(cap, "static/screenshots/03-combat.png");

  for (auto &up : p.upgrades)
    up->level = 0;
  for (auto &up : p.upgrades) {
    if (std::string_view(up->name) == "Staff")
      up->level = 1;
    if (std::string_view(up->name) == "Frost Nova")
      up->level = 1;
  }
  p.lvl = 2;
  p.exp = 0;
  p.expNext = 100;
  p.pendingLevels = 1;
  game.setMenu(std::make_unique<LevelUpMenu>(game));
  for (int i = 0; i < 30; i++) {
    game.tick();
    game.render();
    vk2dRendererPresent();
  }
  snap("static/screenshots/04-upgrades.png");

  vk2dTextureFree(cap);
  game.shutdown();
}
