#include "campaign/Bastion.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include "Math.hpp"

namespace bastion {

namespace {

constexpr int DX[8] = {1, 1, 0, -1, -1, -1, 0, 1}, DY[8] = {0, 1, 1, 1, 0, -1, -1, -1};
constexpr float R2 = 0.70710678f;
constexpr float UX[8] = {1, R2, 0, -R2, -1, -R2, 0, R2}, UY[8] = {0, R2, 1, R2, 0, -R2, -1, -R2};
constexpr float SPAN = 22, DEPTH = 24, EPS = 9, CELL = 16;
constexpr int SIDE_DX[4] = {0, 1, 0, -1}, SIDE_DY[4] = {-1, 0, 1, 0};

std::vector<Field> fields;

bool ring(int cx, int cy, int tx, int ty) {
  int ax = std::abs(tx - cx), ay = std::abs(ty - cy);
  return (ax == W && ay <= H) || (ay == H && ax <= W);
}

} // namespace

int gateOf(int cx, int cy) { return int(hash2(cx * 31 + 7, cy * 17 + 3) & 3); }

Gate gate(int cx, int cy) {
  int s = gateOf(cx, cy);
  double x = cx * CELL + 8.0, y = cy * CELL + 13.0;
  x += SIDE_DX[s] * W * CELL, y += SIDE_DY[s] * H * CELL;
  return {s, x, y, double(SIDE_DX[s]), double(SIDE_DY[s])};
}

bool opening(int cx, int cy, int tx, int ty) {
  int s = gateOf(cx, cy), gx = cx + SIDE_DX[s] * W, gy = cy + SIDE_DY[s] * H;
  return SIDE_DX[s] ? tx == gx && std::abs(ty - gy) <= GAP : ty == gy && std::abs(tx - gx) <= GAP;
}

bool Field::corridor(double x, double y) const {
  double along = gate.nx ? y - gate.y : x - gate.x, across = gate.nx ? x - gate.x : y - gate.y;
  return std::abs(along) < SPAN && std::abs(across) < DEPTH;
}

uint8_t Field::at(double x, double y) const {
  float lx = float(x) - wx0, ly = float(y) - wy0;
  if (!(lx >= 0 && ly >= 0 && lx < N * CELL && ly < N * CELL))
    return NONE;
  return dir[(int(ly) >> 4) * N + (int(lx) >> 4)];
}

void clear() { fields.clear(); }

int count() { return int(fields.size()); }

const Field &field(int i) { return fields[size_t(i)]; }

void raise(int cx, int cy) {
  Field &f = fields.emplace_back();
  f.cx = cx, f.cy = cy, f.x0 = cx - N / 2, f.y0 = cy - N / 2;
  f.wx0 = float(f.x0 * CELL), f.wy0 = float(f.y0 * CELL);
  f.west = float((cx - W) * CELL + 8), f.east = float((cx + W) * CELL + 8);
  f.north = float((cy - H) * CELL + 13), f.south = float((cy + H) * CELL + 13);
  f.gate = gate(cx, cy);
  f.dir.fill(NONE);
  uint16_t queue[N * N];
  int head = 0, tail = 0;
  for (int ly = 0; ly < N; ly++)
    for (int lx = 0; lx < N; lx++) {
      int tx = f.x0 + lx, ty = f.y0 + ly;
      if (!ring(cx, cy, tx, ty))
        continue;
      bool open = opening(cx, cy, tx, ty);
      f.dir[size_t(ly * N + lx)] = open ? GOAL : WALL;
      if (open)
        queue[tail++] = uint16_t(ly * N + lx);
    }
  while (head < tail) {
    int c = queue[head++], cx0 = c % N, cy0 = c / N;
    for (int k = 0; k < 8; k++) {
      int nx = cx0 + DX[k], ny = cy0 + DY[k];
      if (unsigned(nx) >= unsigned(N) || unsigned(ny) >= unsigned(N))
        continue;
      uint8_t &d = f.dir[size_t(ny * N + nx)];
      if (d != NONE)
        continue;
      if ((k & 1) && (f.dir[size_t(cy0 * N + nx)] == WALL || f.dir[size_t(ny * N + cx0)] == WALL))
        continue;
      d = uint8_t((k + 4) & 7);
      queue[tail++] = uint16_t(ny * N + nx);
    }
  }
}

void route(double x, double y, double px, double py, float speed, double &xw, double &yw) {
  for (const Field &f : fields) {
    bool in = f.inside(x, y), hero = f.inside(px, py);
    if (in == hero)
      continue;
    if ((px - x) * xw + (py - y) * yw <= 0)
      return;
    uint8_t d = f.at(x, y);
    if (d < 8)
      xw = UX[d] * speed, yw = UY[d] * speed;
    return;
  }
}

void wall(double ox, double oy, double &x, double &y) {
  for (const Field &f : fields) {
    bool was = f.inside(ox, oy), now = f.inside(x, y);
    if (was == now || f.corridor(ox, oy) || f.corridor(x, y))
      continue;
    if (!now) {
      x = std::clamp(x, double(f.west) + EPS, double(f.east) - EPS);
      y = std::clamp(y, double(f.north) + EPS, double(f.south) - EPS);
      continue;
    }
    double dw = x - f.west, de = f.east - x, dn = y - f.north, ds = f.south - y;
    double m = std::min(std::min(dw, de), std::min(dn, ds));
    if (m == dw)
      x = f.west - EPS;
    else if (m == de)
      x = f.east + EPS;
    else if (m == dn)
      y = f.north - EPS;
    else
      y = f.south + EPS;
  }
}

} // namespace bastion
