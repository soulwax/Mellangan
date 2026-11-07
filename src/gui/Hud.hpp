#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include <SDL3/SDL.h>

#include "gfx/Screen.hpp"

class Game;
namespace world {
struct Passage;
} // namespace world

class Hud {
public:
  std::string banner;
  float bannerTime = 0;

  explicit Hud(Game &game) : game(game) {}
  void announce(std::string text);
  void clear();
  void tick(bool running);
  void render(float now, int xv, int yv);

private:
  struct Pin {
    float x, y, nx, ny;
  };

  Game &game;
  std::deque<std::string> banners;
  std::vector<SDL_FRect> tags;
  float time = 0;
  int xView = 0, yView = 0;

  void draw();
  void lootLabels();
  void experience();
  void stats(float top);
  void night(float top);
  void landmarks();
  void champions();
  void merchant();
  void handwork();
  void questLog();
  void compass();
  void minimap();
  void loadout(float top);
  void statuses(float top);
  void bossBar();
  void heartbeat();
  void vitals();
  SDL_FRect bannerRect() const;
  void bannerBox();
  bool edge(double wx, double wy, Pin &pin) const;
  bool inView(double wx, double wy, float margin) const;
  SDL_FPoint gui(double wx, double wy) const;
  void plate(double wx, double wy, std::string_view text, Color ink);
  void stairs();
  void passage(const world::Passage &w);
  bool crack();
  bool offer();
  void arrow(const Pin &pin, float length, float wing, Color c);
  void diamond(const Pin &pin, Color c);
  void tag(const std::string &text, const Pin &pin, float back, Color c);
};
