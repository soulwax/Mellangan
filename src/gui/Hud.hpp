#pragma once

#include <deque>
#include <string>

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
  Game &game;
  std::deque<std::string> banners;
  float time = 0;
  int xView = 0, yView = 0;

  void draw();
  void landmarks();
  void champions();
  void compass();
  void minimap();
  bool edge(double wx, double wy, float &xe, float &ye, float &xn,
            float &yn) const;
};
