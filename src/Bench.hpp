#pragma once

#include <vector>

class Game;

class Bench {
public:
  bool enabled = false, gate = false, failed = false;
  int soak = 0;

  void start(Game &g);
  void frame(Game &g, double dt, int ticks, double tickSec, double renderSec);
  void interrupt();

private:
  int step = -1, frames = 0, ticks = 0, loads = 0;
  double clock = 0, tickSec = 0, renderSec = 0;
  std::vector<double> costs;
  int minute = 0, runs = 0, heading = 0, slow = 0, stalled = 0;
  double turn = 0, press = 0, firstPeak = 0, laterPeak = 0, runTime = 1e9;

  void enter(Game &g);
  void load(Game &g);
  void drive(Game &g, double dt);
  void endure(Game &g, double dt, int ticks, double tickSec, double renderSec);
};
