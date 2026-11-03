#pragma once

#include <array>
#include <cstdint>

namespace bastion {

constexpr int W = 6, H = 5, N = 64, GAP = 1;
constexpr uint8_t GOAL = 8, WALL = 9, NONE = 255;
enum Side { North, East, South, West };

struct Gate {
  int side;
  double x, y, nx, ny;
};

struct Field {
  int x0, y0, cx, cy;
  float wx0, wy0, west, east, north, south;
  Gate gate;
  std::array<uint8_t, N * N> dir;

  bool inside(double x, double y) const {
    return x > west && x < east && y > north && y < south;
  }
  bool corridor(double x, double y) const;
  uint8_t at(double x, double y) const;
};

int gateOf(int cx, int cy);
Gate gate(int cx, int cy);
bool opening(int cx, int cy, int tx, int ty);
void clear();
void raise(int cx, int cy);
int count();
const Field &field(int i);
void route(double x, double y, double px, double py, float speed, double &xw, double &yw);
void wall(double ox, double oy, double &x, double &y);

} // namespace bastion
