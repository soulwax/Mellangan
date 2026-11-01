#pragma once

#include <deque>
#include <string>

#include "gfx/Screen.hpp"

class Game;

class Hud {
public:
  std::string banner;
  float bannerTime = 0;

  explicit Hud(Game &game) : game(game) {}
  void announce(std::string text);
  void clear();
  void tick(bool running);
  void render(float time, int xView, int yView);

private:
  struct Pin {
    float x, y, nx, ny;
  };

  Game &game;
  std::deque<std::string> banners;
  float time = 0;
  int xView = 0, yView = 0;

  void draw();
  void experience();
  void stats(float top);
  void night(float top);
  void landmarks();
  void champions();
  void compass();
  void minimap();
  void loadout(float top);
  void statuses(float top);
  void bossBar();
  void heartbeat();
  void vitals();
  void bannerBox();
  bool edge(double wx, double wy, Pin &pin) const;
  void arrow(const Pin &pin, float length, float wing, Color c);
  void diamond(const Pin &pin, Color c);
  void tag(const std::string &text, const Pin &pin, float back, Color c);
};
