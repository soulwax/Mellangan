#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "level/Structure.hpp"

class Level;
class Screen;

class WorldDebug {
public:
  WorldDebug(Screen &s, Level &level, int xView, int yView);
  void render(float xMouse, float yMouse);

private:
  typedef std::vector<std::pair<std::string, Color>> Lines;

  Screen &s;
  Level &level;
  int xView, yView;
  float k, u, small, t, xo, yo;
  int rx0, ry0, rx1, ry1;

  float guiX(double x) const { return float(xo + (x - xView) * k); }
  float guiY(double y) const { return float(yo + (y - yView) * k); }
  double worldX(float x) const { return xView + (x - xo) / k; }
  double worldY(float y) const { return yView + (y - yo) / k; }
  bool inside(float x, float y) const;
  std::array<float, 4> box(const Structure &st, int margin) const;

  void regions();
  int structures(const Structure *hovered);
  void target(const Structure &target);
  void hover(int hx, int hy, float xMouse, float yMouse, const Structure *hovered);
  void legend(int seen, const Structure *target);
  void panel(const Lines &lines, float x, float y, bool alignRight);
};
