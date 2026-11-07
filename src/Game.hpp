#pragma once

#include <SDL3/SDL.h>
#include <spdlog/fmt/fmt.h>

#include <deque>
#include <memory>
#include <string>

#include "Bench.hpp"
#include "gfx/Camera.hpp"
#include "DevStart.hpp"
#include "gfx/Screen.hpp"
#include "input/Input.hpp"
#include "campaign/Campaign.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"
#include "meta/Settings.hpp"
#include "meta/Balance.hpp"
#include "meta/Codex.hpp"
#include "meta/Saves.hpp"
#include "gui/Hud.hpp"


inline std::string clockText(float t) {
  return fmt::format("{:02}:{:02}", int(t) / 60, int(t) % 60);
}

inline constexpr char CODE_CHARS[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

inline uint32_t seedSalt(std::string_view code) {
  uint32_t h = 2166136261u;
  for (char c : code)
    h = (h ^ uint8_t(c)) * 16777619u;
  return code.empty() ? 0 : h | 1;
}

namespace world {
enum class Layer : uint8_t;
} // namespace world

class Game {
public:
  SDL_Window *window = nullptr;
  Screen screen;
  Input input;
  std::unique_ptr<Level> level;
  std::unique_ptr<Menu> menu;
  bool running = true, dev = false;
  Settings settings;
  Bench bench;

  WorldMode worldMode = WorldMode::Classic;
  std::string seedCode;
  DevStart start;
  RunRules rules;
  Balance balance;
  Difficulty difficulty = Difficulty::HurtMePlenty;
  int heat = 0;
  bool endless = false;
  float endlessFrom = 0;
  void beginEndless();
  std::string base;
  Saves saves;
  Hud hud{*this};
  Codex codex;
  float best = 0;
  bool beatBest = false, wantsCampaign = false, hardcoreNext = false, changelingNext = false, readOnly = false;
  std::unique_ptr<Campaign> campaign;

  bool init(SDL_Window *window, const std::string &base);
  void shutdown();
  void run();

  // Applied once the current tick is done, so a menu can safely replace itself.
  void setMenu(std::unique_ptr<Menu> m) {
    nextMenu = std::move(m), menuChanged = true;
  }
  void newGame(bool reroll = true);
  void travel(world::Layer to, double x, double y);
  void fall();
  bool crack();
  void toTitle();
  void reseed();
  void snapCamera();
  void loadBalance();
  void setDifficulty(Difficulty d);
  std::string worldName() const;
  void announce(std::string text);
  void applySettings();
  void loadLanguage();
  void saveSettings();
  void saveKeys();
  void observe();
  void checkFeats();
  std::vector<int> earned;
  void onPlayerHurt() { hurt = 1, camera.shake = 4; }
  void onExplosion() { camera.jolt(2.5f); }
  void hitStop(float seconds);
  void celebrate();
  void onPlayerDied();
  void onDawn();
  void beginCampaign();
  bool campaignSaved() const;
  void saveCampaign();
  void endCampaign();
  bool forget();

private:
  std::unique_ptr<Menu> nextMenu;
  bool menuChanged = false, showFps = false, worldDebug = false;
  float time = 0, hurt = 0, fps = 0, freeze = 0,
        lastStop = -1, levelBeat = -1, autosave = Campaign::AUTOSAVE, gatherTime = 0;
  static constexpr float GATHER_REPEAT = 0.3f, FALL = 0.25f;
  RunRules dawnRules;
  std::string chartPath;
  Camera camera;
  int xView = 0, yView = 0;
  float alpha = 1;
  bool stepped = false;
  static constexpr int GRAPH = 120;
  float tickMs[GRAPH] = {}, renderMs[GRAPH] = {};
  int graphAt = 0;

  void tick();
  void devTick();
  bool writes() const { return !saves.dir().empty() && !bench.enabled && !readOnly; }
  void hotkeys();
  bool route();
  std::unique_ptr<Menu> nearby();
  void play();
  void place();
  void gather();
  void stairs();
  std::string placeName() const;
  void keep();
  void bankLevels();
  void build();
  void loadWorld();
  void saveWorld() const;
  void remember(bool dawn);
  void render();
  void frameGraph(float x, float y);
  friend void captureScreenshotSuite(SDL_Window *window, const std::string &base);
};
