#pragma once

#include "gfx/Screen.hpp"

class Sky;

class FloorPainter {
public:
  FloorPainter(Screen &s, const Sky &sky);
  void tile(int x, int y);

private:
  Screen &s;
  const Sky &sky;
  bool snowing, flooded;
  float dry = 0.5f;

  void tint(Color t, float shade, float n, float alpha = 1);
  void blend(int x, int y, int own, float shade, float n);
  void edge(int x, int y, int own, bool water);
  void pool(int x, int y, float shade);
  void settle(int x, int y, float shade);
  void wall(int x, int y, float shade);
  void cliff(int x, int y, int level, float shade);
};
