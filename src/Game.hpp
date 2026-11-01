#pragma once

#include <SDL3/SDL.h>
#include <spdlog/fmt/fmt.h>

#include <deque>
#include <memory>
#include <string>

#include "Bench.hpp"
#include "DevStart.hpp"
#include "gfx/Screen.hpp"
#include "input/Input.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"
#include "meta/Balance.hpp"
#include "meta/Codex.hpp"
#include "meta/Saves.hpp"
#include "gui/Hud.hpp"


inline std::string clockText(float t) {
  return fmt::format("{:02}:{:02}", int(t) / 60, int(t) % 60);
}

class Game {
public:
  SDL_Window *window = nullptr;
  Screen screen;
  Input input;
  std::unique_ptr<Level> level;
  std::unique_ptr<Menu> menu;
  bool running = true, dev = false, reducedParticles = false;
  Bench bench;

  WorldMode worldMode = WorldMode::Classic;
  std::string seedCode;
  DevStart start;
  Balance balance;
  Difficulty difficulty = Difficulty::HurtMePlenty;
  std::string base;
  Saves saves;
  Hud hud{*this};
  Codex codex;
  float best = 0;
  bool beatBest = false;

  bool init(SDL_Window *window, const std::string &base);
  void shutdown();
  void run();

  // Applied once the current tick is done, so a menu can safely replace itself.
  void setMenu(std::unique_ptr<Menu> m) {
    nextMenu = std::move(m), menuChanged = true;
  }
  void newGame(bool reroll = true);
  void toTitle();
  void reseed();
  void snapCamera();
  void loadBalance();
  void setDifficulty(Difficulty d);
  std::string worldName() const;
  void announce(std::string text);
  void setReducedParticles(bool reduced);
  void onPlayerHurt() { hurt = 1, shake = 4; }
  void onExplosion() { shake = std::max(shake, 2.5f); }
  void hitStop(float seconds);
  void celebrate();
  void onPlayerDied();
  void onDawn();

private:
  std::unique_ptr<Menu> nextMenu;
  bool menuChanged = false, showFps = false, worldDebug = false;
  float time = 0, hurt = 0, shake = 0, fps = 0, freeze = 0,
        lastStop = -1, levelBeat = -1;
  std::string chartPath;
  double xCam = 0, yCam = 0;
  int xView = 0, yView = 0;

  void tick();
  void devTick();
  void build();
  void loadWorld();
  void saveWorld() const;
  void remember(bool dawn);
  void render();
  friend void captureScreenshotSuite(SDL_Window *window, const std::string &base);
};
