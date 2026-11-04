#pragma once

#include <vector>

class Game;

class Bench {
public:
  bool enabled = false, gate = false, failed = false;

  void start(Game &g);
  void frame(Game &g, double dt, int ticks, double tickSec, double renderSec);

private:
  int step = -1, frames = 0, ticks = 0, loads = 0;
  double clock = 0, tickSec = 0, renderSec = 0;
  std::vector<double> costs;

  void enter(Game &g);
  void load(Game &g);
};
