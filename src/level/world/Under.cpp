#include "level/world/Internal.hpp"

#include <deque>
#include <vector>

#include "level/Edits.hpp"

namespace world {

Layer active = Layer::Surface;
bool mouthsOpen = false;

Layer layer() { return active; }

void enter(Layer to) {
  if (to == active)
    return;
  active = to;
  noise_epoch++;
}

namespace {

struct Surfaced {
  Layer was = active;
  Surfaced() { active = Layer::Surface; }
  ~Surfaced() { active = was; }
};

constexpr int MARGIN = 3, MIN_SIZE = 200, ROCK = 3, VAULT_ODDS = 60, VAULT_R2 = 5, LAIR_ODDS = 16;

struct Cell {
  CaveCell info;
  uint32_t epoch = 0;
  uint64_t open[CAVE_CELL];
};
Cell cells[64];

struct CaveMemo {
  int32_t x, y;
  uint32_t epoch;
  uint8_t has, level, raw, wet;
};
CaveMemo memos[128 * 64];

int floorDiv(int v, int d) { return (v - (v < 0 ? d - 1 : 0)) / d; }

CaveKind kindOf(uint32_t h) {
  int r = int(h % 100);
  if (r < 40)
    return CaveKind::Solid;
  r = (r - 40) * 100 / 60;
  return r < 55 ? CaveKind::Pocket : r < 80 ? CaveKind::Through : r < 92 ? CaveKind::Warren
                                                                       : CaveKind::Descent;
}

struct Room {
  float x, y, ra, rb, ca, sa;
};

struct Plan {
  Room room[4];
  int rooms, links[4][2], linked;
};

Plan plan(CaveKind kind, uint32_t h, int x0, int y0) {
  auto unit = [&](int salt) { return float(hash2(int(h) + salt * 7919, salt * 104729 + 3) & 1023) / 1023; };
  Plan p{};
  float cx = x0 + CAVE_CELL * 0.5f, cy = y0 + CAVE_CELL * 0.5f;
  auto room = [&](float x, float y, float ra, float rb, int salt) {
    float a = unit(salt) * float(TAU);
    p.room[p.rooms++] = {x, y, ra, rb, std::cos(a), std::sin(a)};
  };
  if (kind == CaveKind::Pocket) {
    room(cx + (unit(1) - 0.5f) * 8, cy + (unit(2) - 0.5f) * 8, 14 + 6 * unit(3), 11 + 5 * unit(4), 5);
  } else if (kind == CaveKind::Through) {
    float a = unit(1) * float(TAU), half = 16 + 5 * unit(2), dx = std::cos(a) * half, dy = std::sin(a) * half;
    room(cx - dx, cy - dy, 7 + 3 * unit(3), 6 + 3 * unit(4), 5);
    room(cx + dx, cy + dy, 7 + 3 * unit(6), 6 + 3 * unit(7), 8);
  } else {
    int n = kind == CaveKind::Warren ? 3 + int(unit(1) > 0.5f) : 2 + int(unit(1) > 0.5f);
    for (int r = 0; r < n; r++)
      room(x0 + 14 + unit(10 + r) * 36, y0 + 14 + unit(20 + r) * 36, 8 + 5 * unit(30 + r),
           7 + 4 * unit(40 + r), 50 + r);
  }
  for (int r = 1; r < p.rooms; r++)
    p.links[p.linked][0] = r - 1, p.links[p.linked++][1] = r;
  if (p.rooms == 4 && unit(60) > 0.5f)
    p.links[p.linked][0] = 0, p.links[p.linked++][1] = 2;
  return p;
}

bool carved(int x, int y, const Plan &p, uint32_t salt) {
  for (int r = 0; r < p.rooms; r++) {
    const Room &m = p.room[r];
    float dx = x - m.x, dy = y - m.y, u = dx * m.ca + dy * m.sa, v = dy * m.ca - dx * m.sa;
    float e = (u * u) / (m.ra * m.ra) + (v * v) / (m.rb * m.rb) +
              0.35f * noise_fbm(x * 0.08f + 3.7f, y * 0.08f - 1.9f, 2, 3011 + salt);
    if (e < 1 && (dx * dx + dy * dy < 12 ||
                  noise_fbm(x * 0.07f - 12.3f, y * 0.07f + 5.1f, 3, 3001 + salt) > 0.1f - 0.6f * (1 - e)))
      return true;
  }
  float wx = x + 5 * noise_fbm(x * 0.06f + 1.3f, y * 0.06f + 7.1f, 2, 3041 + salt),
        wy = y + 5 * noise_fbm(x * 0.06f - 5.9f, y * 0.06f + 2.2f, 2, 3043 + salt);
  for (int l = 0; l < p.linked; l++) {
    const Room &a = p.room[p.links[l][0]], &b = p.room[p.links[l][1]];
    float ex = b.x - a.x, ey = b.y - a.y, len = ex * ex + ey * ey;
    float t = len > 0 ? clampf(((wx - a.x) * ex + (wy - a.y) * ey) / len, 0, 1) : 0;
    float qx = wx - (a.x + ex * t), qy = wy - (a.y + ey * t);
    float width = 2 + 0.6f * (noise_fbm(x * 0.15f + 9.1f, y * 0.15f - 4.4f, 2, 3045 + salt) + 1);
    if (qx * qx + qy * qy < width * width)
      return true;
  }
  return false;
}

bool dryAbove(int x, int y) {
  Surfaced up;
  Structure s;
  return !isWater(x, y) && !structureAt(x, y, s, 2);
}

void build(Cell *c, int cx, int cy) {
  uint32_t h = hash2(cx * 73 + 11, cy * 151 - 7);
  CaveCell &k = c->info;
  k = {cx, cy, kindOf(h), 0, 0, 0, 0, {}, {}, 0, 0, CaveBiome::Roots, false, 0, 0, 0, 0};
  std::fill(std::begin(c->open), std::end(c->open), 0ull);
  int x0 = cx * CAVE_CELL, y0 = cy * CAVE_CELL;
  Plan shape = plan(k.kind, h, x0, y0);
  k.hx = std::clamp(int(shape.room[0].x), x0 + MARGIN + 2, x0 + CAVE_CELL - MARGIN - 3);
  k.hy = std::clamp(int(shape.room[0].y), y0 + MARGIN + 2, y0 + CAVE_CELL - MARGIN - 3);
  {
    Surfaced up;
    k.biome = caveBelow(world::biomeAt(k.hx, k.hy));
  }
  if (k.kind == CaveKind::Solid)
    return;
  static bool raw[CAVE_CELL][CAVE_CELL];
  for (int j = 0; j < CAVE_CELL; j++)
    for (int i = 0; i < CAVE_CELL; i++)
      raw[j][i] = i >= MARGIN && j >= MARGIN && i < CAVE_CELL - MARGIN &&
                  j < CAVE_CELL - MARGIN &&
                  ((x0 + i - k.hx) * (x0 + i - k.hx) + (y0 + j - k.hy) * (y0 + j - k.hy) < 9 ||
                   carved(x0 + i, y0 + j, shape, 0));
  auto rawAt = [&](int i, int j) {
    return i >= 0 && j >= 0 && i < CAVE_CELL && j < CAVE_CELL && raw[j][i];
  };
  auto core = [&](int i, int j) {
    if (!rawAt(i, j))
      return false;
    for (int dj = -1; dj <= 0; dj++)
      for (int di = -1; di <= 0; di++)
        if (rawAt(i + di, j + dj) && rawAt(i + di + 1, j + dj) && rawAt(i + di, j + dj + 1) &&
            rawAt(i + di + 1, j + dj + 1))
          return true;
    return false;
  };
  static bool keep[CAVE_CELL][CAVE_CELL];
  for (int j = 0; j < CAVE_CELL; j++)
    for (int i = 0; i < CAVE_CELL; i++)
      keep[j][i] = core(i, j);
  static int dist[CAVE_CELL][CAVE_CELL];
  for (auto &row : dist)
    std::fill(std::begin(row), std::end(row), -1);
  std::vector<int> queue{(k.hy - y0) * CAVE_CELL + k.hx - x0};
  dist[k.hy - y0][k.hx - x0] = 0;
  int far = queue[0];
  for (size_t q = 0; q < queue.size(); q++) {
    int i = queue[q] % CAVE_CELL, j = queue[q] / CAVE_CELL;
    c->open[j] |= 1ull << i;
    if (dist[j][i] > dist[far / CAVE_CELL][far % CAVE_CELL])
      far = queue[q];
    constexpr int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto &d : D) {
      int ni = i + d[0], nj = j + d[1];
      if (ni >= 0 && nj >= 0 && ni < CAVE_CELL && nj < CAVE_CELL && keep[nj][ni] &&
          dist[nj][ni] < 0)
        dist[nj][ni] = dist[j][i] + 1, queue.push_back(nj * CAVE_CELL + ni);
    }
  }
  k.size = int(queue.size());
  auto clear = [&] {
    k.kind = CaveKind::Solid, k.size = 0, k.mouths = 0;
    std::fill(std::begin(c->open), std::end(c->open), 0ull);
  };
  if (k.size < MIN_SIZE)
    return clear();
  auto openAt = [&](int i, int j) {
    return i >= 0 && j >= 0 && i < CAVE_CELL && j < CAVE_CELL && (c->open[j] >> i & 1);
  };
  std::vector<int> spots;
  for (int j = 1; j < CAVE_CELL - 1; j += 3)
    for (int i = 1; i < CAVE_CELL - 1; i += 3) {
      bool roomy = shape.rooms > 1 || dist[j][i] >= 6;
      for (int dj = -1; dj <= 1 && roomy; dj++)
        for (int di = -1; di <= 1 && roomy; di++)
          roomy = openAt(i + di, j + dj);
      if (roomy && dryAbove(x0 + i, y0 + j))
        spots.push_back(j * CAVE_CELL + i);
    }
  if (spots.empty())
    return clear();
  int want = k.kind == CaveKind::Pocket || k.kind == CaveKind::Descent ? 1
             : k.kind == CaveKind::Through                            ? 2
                                                                      : 2 + int(h >> 30 & 1);
  auto place = [&](int spot) {
    k.mx[k.mouths] = x0 + spot % CAVE_CELL, k.my[k.mouths] = y0 + spot / CAVE_CELL, k.mouths++;
  };
  for (int m = 0; m < want && m < shape.rooms; m++) {
    const Room &r = shape.room[shape.rooms - 1 - m];
    float a = float(hash2(int(h), m) & 1023) / 1023 * float(TAU), tx = r.x + std::cos(a) * 3,
          ty = r.y + std::sin(a) * 3;
    int best = -1;
    double near = 1e9;
    for (int s : spots) {
      double dx = x0 + s % CAVE_CELL - tx, dy = y0 + s / CAVE_CELL - ty, d = dx * dx + dy * dy;
      for (int o = 0; o < k.mouths; o++) {
        double ox = x0 + s % CAVE_CELL - k.mx[o], oy = y0 + s / CAVE_CELL - k.my[o];
        if (ox * ox + oy * oy < 144)
          d = 1e9;
      }
      if (d < near)
        near = d, best = s;
    }
    if (best >= 0 && near < 1e9)
      place(best);
  }
  if (!k.mouths) {
    int first = spots[0];
    for (int s : spots)
      if (hash2(s, int(h)) > hash2(first, int(h)))
        first = s;
    place(first);
  }
  while (k.mouths < want) {
    int best = -1;
    double gap = -1;
    for (int s : spots) {
      double near = 1e9;
      for (int m = 0; m < k.mouths; m++) {
        double dx = x0 + s % CAVE_CELL - k.mx[m], dy = y0 + s / CAVE_CELL - k.my[m];
        near = std::min(near, dx * dx + dy * dy);
      }
      if (near > gap)
        gap = near, best = s;
    }
    if (best < 0 || gap < 144)
      break;
    place(best);
  }
  if (k.kind == CaveKind::Through && k.mouths < 2)
    k.kind = CaveKind::Pocket;
  if (k.kind == CaveKind::Descent)
    k.px = x0 + far % CAVE_CELL, k.py = y0 + far / CAVE_CELL;
  if ((h >> 12) % 100 >= VAULT_ODDS)
    return;
  uint32_t best = 0;
  for (int j = MARGIN; j < CAVE_CELL - MARGIN; j++)
    for (int i = MARGIN; i < CAVE_CELL - MARGIN; i++) {
      if (!openAt(i, j) || (x0 + i - k.hx) * (x0 + i - k.hx) + (y0 + j - k.hy) * (y0 + j - k.hy) <= 36)
        continue;
      constexpr int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
      for (int d = 0; d < 4; d++) {
        int ci = i + 4 * D[d][0], cj = j + 4 * D[d][1];
        bool sealed = !openAt(i + D[d][0], j + D[d][1]) && ci >= 3 && cj >= 3 && ci < CAVE_CELL - 3 &&
                      cj < CAVE_CELL - 3;
        for (int dj = -3; dj <= 3 && sealed; dj++)
          for (int di = -3; di <= 3 && sealed; di++)
            sealed = di * di + dj * dj > 10 || !openAt(ci + di, cj + dj);
        if (uint32_t s = hash2(x0 + i * 4 + d, y0 + j * 7 + int(h & 1023)); sealed && s > best)
          best = s, k.vault = true, k.kx = x0 + i + D[d][0], k.ky = y0 + j + D[d][1], k.vx = x0 + ci,
          k.vy = y0 + cj;
    }
    }
  if (!k.vault)
    return;
  for (int dj = -2; dj <= 2; dj++)
    for (int di = -2; di <= 2; di++)
      if (di * di + dj * dj <= VAULT_R2)
        c->open[k.vy - y0 + dj] |= 1ull << (k.vx - x0 + di);
}

const Cell &cellOf(int cx, int cy) {
  Cell *c = &cells[(cx & 7) | (cy & 7) << 3];
  if (c->epoch != noise_epoch || c->info.cx != cx || c->info.cy != cy) {
    build(c, cx, cy);
    c->epoch = noise_epoch;
  }
  return *c;
}

struct Deep {
  DeepCell info;
  uint32_t epoch = 0;
  uint64_t open[CAVE_CELL], spine[CAVE_CELL];
};
Deep deeps[64];

struct Climb {
  CaveBiome biome;
  Way way;
  uint32_t odds;
};
constexpr Climb CLIMBS[] = {
    {CaveBiome::Roots, Way::Roots, 50}, {CaveBiome::Vents, Way::Chimney, 50}, {CaveBiome::Catacombs, Way::Stair, 70}};

int rawNoise(int x, int y, bool below) {
  float n = noise_fbm(x * 0.045f + (below ? 91.3f : 7.7f), y * 0.045f - 3.3f, 2, 3021);
  return n > 0.38f ? 2 : n > 0.12f ? 1 : 0;
}

float poolNoise(int x, int y, bool below) {
  return noise_fbm(x * 0.05f - (below ? 63.1f : 21.7f), y * 0.05f + 14.2f, 3, 3031);
}

int gate(int cx, int cy, int salt) { return 10 + int(hash2(cx * 31 + salt, cy * 17 - salt) % 44); }

bool sinkhole(int x0, int y0, uint32_t h, int &wx, int &wy) {
  Surfaced up;
  Structure s;
  constexpr int RING[8][2] = {{4, 0}, {-4, 0}, {0, 4}, {0, -4}, {3, 3}, {-3, 3}, {3, -3}, {-3, -3}};
  for (int n = 0; n < 36; n++) {
    int k = int((n + h) % 36), x = x0 + 10 + k % 6 * 8 + int(h >> 8 & 3), y = y0 + 10 + k / 6 * 8 + int(h >> 10 & 3);
    if (isWater(x, y) || heightAt(x, y) != 0 || structureAt(x, y, s, 2))
      continue;
    for (const auto &r : RING)
      if (isWater(x + r[0], y + r[1]))
        return wx = x, wy = y, true;
  }
  return false;
}

void dig(Deep *d, int cx, int cy) {
  constexpr int C = CAVE_CELL, N = C * C;
  uint32_t h = hash2(cx * 197 - 5, cy * 61 + 13);
  auto unit = [&](int salt) { return float(hash2(int(h) + salt * 7919, salt * 104729 + 5) & 1023) / 1023; };
  int x0 = cx * C, y0 = cy * C;
  Plan shape{};
  shape.rooms = 3 + int(unit(1) > 0.5f);
  for (int r = 0; r < shape.rooms; r++) {
    float a = unit(70 + r) * float(TAU);
    shape.room[r] = {x0 + 14 + unit(10 + r) * 36, y0 + 14 + unit(20 + r) * 36, 11 + 8 * unit(30 + r),
                     9 + 7 * unit(40 + r), std::cos(a), std::sin(a)};
  }
  for (int r = 1; r < shape.rooms; r++)
    shape.links[shape.linked][0] = r - 1, shape.links[shape.linked++][1] = r;
  if (shape.rooms == 4 && unit(60) > 0.5f)
    shape.links[shape.linked][0] = 0, shape.links[shape.linked++][1] = 3;
  DeepCell &k = d->info;
  float heat = noise_fbm(cx * 0.23f + 4.1f, cy * 0.23f - 8.3f, 2, 3101),
        damp = noise_fbm(cx * 0.23f - 6.7f, cy * 0.23f + 2.9f, 2, 3103),
        age = noise_fbm(cx * 0.19f + 1.3f, cy * 0.19f + 9.4f, 2, 3105);
  k = {cx, cy, 0, std::clamp(int(shape.room[0].x), x0 + MARGIN + 2, x0 + C - MARGIN - 3),
       std::clamp(int(shape.room[0].y), y0 + MARGIN + 2, y0 + C - MARGIN - 3), deepClimate(heat, damp, age),
       false, 0, 0, Way::None, 0, 0, false};
  k.lair = CAVES[int(k.biome)].guardian && (h >> 9) % 100 < LAIR_ODDS;
  const CaveCell above = cellOf(cx, cy).info;
  if (above.kind == CaveKind::Descent)
    k.landing = true, k.lx = above.px, k.ly = above.py;
  uint32_t roll = hash2(cx * 41 - 9, cy * 67 + 5) % 100;
  if (above.kind != CaveKind::Solid) {
    for (const Climb &c : CLIMBS)
      if (c.biome == above.biome && roll < c.odds)
        k.way = c.way, k.wx = above.hx + 2, k.wy = above.hy;
    for (int m = 0; m < above.mouths; m++)
      if (std::abs(above.mx[m] - k.wx) <= 2 && std::abs(above.my[m] - k.wy) <= 2)
        k.way = Way::None;
    if (k.landing && (k.lx - k.wx) * (k.lx - k.wx) + (k.ly - k.wy) * (k.ly - k.wy) <= 36)
      k.way = Way::None;
  } else if (roll < 40 && sinkhole(x0, y0, h, k.wx, k.wy)) {
    k.way = Way::Sinkhole;
  }
  int ge = gate(cx, cy, 7), gw = gate(cx - 1, cy, 7), gs = gate(cx, cy, 13), gn = gate(cx, cy - 1, 13);
  auto disc = [&](int x, int y, int i, int j, int r2) {
    return (x0 + i - x) * (x0 + i - x) + (y0 + j - y) * (y0 + j - y) <= r2;
  };
  static bool keep[C][C], fixed[C][C], core[C][C];
  for (int j = 0; j < C; j++)
    for (int i = 0; i < C; i++) {
      bool stub = (i >= C - 5 && std::abs(j - ge) <= 1) || (i <= 4 && std::abs(j - gw) <= 1) ||
                  (j >= C - 5 && std::abs(i - gs) <= 1) || (j <= 4 && std::abs(i - gn) <= 1);
      bool pad = disc(k.hx, k.hy, i, j, 8) || (k.landing && disc(k.lx, k.ly, i, j, 16)) ||
                 (k.way != Way::None && disc(k.wx, k.wy, i, j, 6));
      bool inner = i >= MARGIN && j >= MARGIN && i < C - MARGIN && j < C - MARGIN;
      fixed[j][i] = stub || pad;
      keep[j][i] = inner && noise_fbm((x0 + i) * 0.21f + 2.2f, (y0 + j) * 0.21f - 7.6f, 2, 3201) < 0.45f &&
                   carved(x0 + i, y0 + j, shape, 200);
    }
  auto rawAt = [&](int i, int j) { return i >= 0 && j >= 0 && i < C && j < C && keep[j][i]; };
  for (int j = 0; j < C; j++)
    for (int i = 0; i < C; i++) {
      core[j][i] = fixed[j][i];
      for (int dj = -1; dj <= 0 && !core[j][i] && rawAt(i, j); dj++)
        for (int di = -1; di <= 0; di++)
          core[j][i] = core[j][i] || (rawAt(i + di, j + dj) && rawAt(i + di + 1, j + dj) &&
                                      rawAt(i + di, j + dj + 1) && rawAt(i + di + 1, j + dj + 1));
    }
  static int from[N];
  int heart = (k.hy - y0) * C + k.hx - x0;
  std::vector<int> queue;
  auto flood = [&] {
    std::fill(std::begin(from), std::end(from), -1);
    queue.assign(1, heart);
    from[heart] = heart;
    for (size_t q = 0; q < queue.size(); q++) {
      int i = queue[q] % C, j = queue[q] / C;
      constexpr int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
      for (const auto &o : D) {
        int ni = i + o[0], nj = j + o[1];
        if (ni >= 0 && nj >= 0 && ni < C && nj < C && core[nj][ni] && from[nj * C + ni] < 0)
          from[nj * C + ni] = queue[q], queue.push_back(nj * C + ni);
      }
    }
  };
  auto local = [&](int x, int y) { return std::clamp(y - y0, 0, C - 1) * C + std::clamp(x - x0, 0, C - 1); };
  int targets[6], n = 0;
  targets[n++] = ge * C + C - 1, targets[n++] = gw * C, targets[n++] = (C - 1) * C + gs, targets[n++] = gn;
  if (k.landing)
    targets[n++] = local(k.lx, k.ly);
  if (k.way != Way::None)
    targets[n++] = local(k.wx, k.wy);
  flood();
  for (int t = 0; t < n; t++) {
    if (from[targets[t]] >= 0)
      continue;
    int ti = targets[t] % C, tj = targets[t] / C, best = heart, gap = 1 << 30;
    for (int c : queue)
      if (int g = (c % C - ti) * (c % C - ti) + (c / C - tj) * (c / C - tj); g < gap)
        gap = g, best = c;
    int dx = best % C - ti, dy = best / C - tj, steps = std::max(std::abs(dx), std::abs(dy));
    for (int s = 0; s <= steps; s++) {
      int i = ti + (steps ? dx * s / steps : 0), j = tj + (steps ? dy * s / steps : 0);
      for (int dj = -1; dj <= 1; dj++)
        for (int di = -1; di <= 1; di++)
          if (i + di >= 0 && j + dj >= 0 && i + di < C && j + dj < C)
            core[j + dj][i + di] = true;
    }
    flood();
  }
  std::fill(std::begin(d->open), std::end(d->open), 0ull);
  std::fill(std::begin(d->spine), std::end(d->spine), 0ull);
  for (int c : queue)
    d->open[c / C] |= 1ull << (c % C);
  auto mark = [&](int c) {
    for (int dj = -1; dj <= 1; dj++)
      for (int di = -1; di <= 1; di++)
        if (int i = c % C + di, j = c / C + dj; i >= 0 && j >= 0 && i < C && j < C)
          d->spine[j] |= 1ull << i;
  };
  for (int t = 0; t < n; t++)
    for (int c = targets[t]; c != heart; c = from[c])
      mark(c);
  for (int j = 0; j < C; j++)
    for (int i = 0; i < C; i++)
      if (fixed[j][i])
        d->spine[j] |= 1ull << i;
  k.size = int(queue.size());
  if (CAVES[int(k.biome)].pool != CavePool::Lava)
    return;
  static bool hot[N], done[N];
  static int cost[N];
  for (int c = 0; c < N; c++) {
    int i = c % C, j = c / C;
    bool padded = disc(k.hx, k.hy, i, j, 16) || (k.landing && disc(k.lx, k.ly, i, j, 16)) ||
                  (k.way != Way::None && disc(k.wx, k.wy, i, j, 16));
    hot[c] = from[c] >= 0 && !padded && rawNoise(x0 + i, y0 + j, true) == 0 &&
             poolNoise(x0 + i, y0 + j, true) < CAVES[int(k.biome)].pools;
    cost[c] = 1 << 30, done[c] = false;
  }
  std::deque<int> todo{heart};
  cost[heart] = 0, from[heart] = heart;
  while (!todo.empty()) {
    int c = todo.front();
    todo.pop_front();
    constexpr int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    for (const auto &o : D) {
      int i = c % C + o[0], j = c / C + o[1], n2 = j * C + i;
      if (i < 0 || j < 0 || i >= C || j >= C || !(d->open[j] >> i & 1))
        continue;
      int step = hot[n2] && !(d->spine[j] >> i & 1);
      if (cost[c] + step >= cost[n2])
        continue;
      cost[n2] = cost[c] + step, from[n2] = c;
      step ? todo.push_back(n2) : todo.push_front(n2);
    }
  }
  for (int c = 0; c < N; c++)
    for (int t = c; !hot[c] && cost[t] > 0 && cost[t] < (1 << 30) && !done[t]; t = from[t]) {
      done[t] = true;
      if (hot[t])
        d->spine[t / C] |= 1ull << (t % C);
    }
}

const Deep &deepOf(int cx, int cy) {
  Deep *d = &deeps[(cx & 7) | (cy & 7) << 3];
  if (d->epoch != noise_epoch || d->info.cx != cx || d->info.cy != cy) {
    dig(d, cx, cy);
    d->epoch = noise_epoch;
  }
  return *d;
}

bool deep() { return active == Layer::Deep; }

bool broken(int x, int y) {
  const Edits::Change *e = edits ? edits->at(x, y) : nullptr;
  return e && !e->placed;
}

bool openAt(int x, int y) {
  int cx = floorDiv(x, CAVE_CELL), cy = floorDiv(y, CAVE_CELL), i = x - cx * CAVE_CELL, j = y - cy * CAVE_CELL;
  if (deep())
    return deepOf(cx, cy).open[j] >> i & 1;
  const Cell &c = cellOf(cx, cy);
  return (c.open[j] >> i & 1) || (c.info.vault && x == c.info.kx && y == c.info.ky && broken(x, y));
}

bool spared(int x, int y, int r2) {
  int cx = floorDiv(x, CAVE_CELL), cy = floorDiv(y, CAVE_CELL);
  auto near = [&](int px, int py) { return (x - px) * (x - px) + (y - py) * (y - py) <= r2; };
  if (deep()) {
    const Deep &d = deepOf(cx, cy);
    const DeepCell &k = d.info;
    return (d.spine[y - cy * CAVE_CELL] >> (x - cx * CAVE_CELL) & 1) || near(k.hx, k.hy) ||
           (k.landing && near(k.lx, k.ly)) || (k.way != Way::None && near(k.wx, k.wy));
  }
  const CaveCell &c = cellOf(cx, cy).info;
  if (near(c.hx, c.hy) || isChasm(x, y) || (c.vault && (near(c.vx, c.vy) || near(c.kx, c.ky))))
    return true;
  for (int m = 0; m < c.mouths; m++)
    if (near(c.mx[m], c.my[m]))
      return true;
  return false;
}

CaveMemo *memoOf(int x, int y) {
  CaveMemo *m = &memos[(x & 127) | (y & 63) << 7];
  if (m->epoch != noise_epoch || m->x != x || m->y != y)
    m->x = x, m->y = y, m->epoch = noise_epoch, m->has = 0;
  return m;
}

int rawLevel(int x, int y) {
  CaveMemo *m = memoOf(x, y);
  if (!(m->has & 1)) {
    m->raw = uint8_t(rawNoise(x, y, deep())), m->has |= 1;
  }
  return m->raw;
}

bool rawAbove(int x, int y, const void *k) {
  return openAt(x, y) && rawLevel(x, y) >= *static_cast<const int *>(k);
}

constexpr Color GLOW[] = {{0, 0, 0, 1},          {0.36f, 0.34f, 0.28f, 1}, {0.3f, 0.3f, 0.36f, 1},
                          {0.22f, 0.36f, 0.18f, 1}, {0.6f, 0.24f, 0.06f, 1},  {0.44f, 0.42f, 0.32f, 1}};

bool wayAt(int x, int y, Passage &out) {
  if (caveMouth(x, y))
    return out = {x, y, active == Layer::Surface ? Layer::Caves : Layer::Surface, Way::Mouth}, true;
  if (active == Layer::Surface)
    return false;
  const DeepCell &d = deepOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
  if (d.way == Way::None || d.wx != x || d.wy != y || (active == Layer::Caves && d.way != Way::Stair))
    return false;
  Layer to = active == Layer::Caves ? Layer::Deep : d.way == Way::Sinkhole ? Layer::Surface : Layer::Caves;
  return out = {x, y, to, d.way}, true;
}

} // namespace

bool caveCell(int cx, int cy, CaveCell &out) {
  out = cellOf(cx, cy).info;
  return out.kind != CaveKind::Solid;
}

bool deepCell(int cx, int cy, DeepCell &out) {
  out = deepOf(cx, cy).info;
  return out.size > 0;
}

bool isRock(int x, int y) { return active != Layer::Surface && !openAt(x, y); }

bool isChasm(int x, int y) {
  if (active != Layer::Caves)
    return false;
  const CaveCell &c = cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
  return c.kind == CaveKind::Descent && (x - c.px) * (x - c.px) + (y - c.py) * (y - c.py) <= 5 && openAt(x, y) &&
         !inVault(x, y);
}

bool crackAt(int x, int y) {
  if (active != Layer::Caves)
    return false;
  const CaveCell &c = cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
  return c.vault && x == c.kx && y == c.ky && !broken(x, y);
}

bool crackNear(double x, double y, int &tx, int &ty) {
  int cx = int(std::floor(x / 16)), cy = int(std::floor((y - 4) / 16));
  for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++)
      if (crackAt(cx + dx, cy + dy))
        return tx = cx + dx, ty = cy + dy, true;
  return false;
}

bool inVault(int x, int y) {
  if (active != Layer::Caves)
    return false;
  const CaveCell &c = cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
  return c.vault && (x - c.vx) * (x - c.vx) + (y - c.vy) * (y - c.vy) <= VAULT_R2;
}

CaveBiome caveBiomeAt(int x, int y) {
  int cx = floorDiv(x, CAVE_CELL), cy = floorDiv(y, CAVE_CELL);
  return deep() ? deepOf(cx, cy).info.biome : cellOf(cx, cy).info.biome;
}

bool isLava(int x, int y) {
  return active != Layer::Surface && under::pool(x, y) &&
         CAVES[int(caveBiomeAt(x, y))].pool == CavePool::Lava;
}

void openCaves(bool open) {
  if (open != mouthsOpen)
    mouthsOpen = open, noise_epoch++;
}

bool caveMouth(int x, int y) {
  if (active == Layer::Deep || (active == Layer::Surface && !mouthsOpen))
    return false;
  const CaveCell &c = cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
  for (int m = 0; m < c.mouths; m++)
    if (c.mx[m] == x && c.my[m] == y)
      return true;
  return false;
}

bool caveMouthNear(double x, double y, int &tx, int &ty) {
  int cx = int(std::floor(x / 16)), cy = int(std::floor((y - 4) / 16));
  for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++)
      if (caveMouth(cx + dx, cy + dy))
        return tx = cx + dx, ty = cy + dy, true;
  return false;
}

bool passageNear(double x, double y, Passage &out) {
  int cx = int(std::floor(x / 16)), cy = int(std::floor((y - 4) / 16));
  for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++)
      if (wayAt(cx + dx, cy + dy, out))
        return true;
  return false;
}

bool mouthProp(int x, int y, Prop &p) {
  Passage w;
  if (!wayAt(x, y, w))
    return false;
  constexpr Sprite LOOK[] = {atlas::tile(39), atlas::tile(39), atlas::tile(39), atlas::under(atlas::ug::Roots),
                             atlas::under(atlas::ug::Chimney), atlas::under(atlas::ug::Sinkhole)};
  p.add(LOOK[int(w.way)]);
  if (active != Layer::Surface)
    p.fire(GLOW[int(w.way)], 120, -4);
  return true;
}

namespace under {

BiomeId biomeAt(int x, int y) {
  Surfaced up;
  return world::biomeAt(x, y);
}

GroundBlend groundBlend(int x, int y) { return {{biomeAt(x, y)}, {1}, 1}; }

int heightAt(int x, int y) {
  if (!openAt(x, y))
    return ROCK;
  CaveMemo *m = memoOf(x, y);
  if (!(m->has & 2)) {
    int level = 0;
    for (int k = 1; k <= 2 && smoothPatch(rawAbove, &k, x, y); k++)
      level = k;
    m = memoOf(x, y);
    m->level = uint8_t(level), m->has |= 2;
  }
  return m->level;
}

bool isDirt(int, int) { return false; }

bool pool(int x, int y) {
  if (deep() ? !openAt(x, y) || rawLevel(x, y) != 0 : heightAt(x, y) != 0)
    return false;
  CaveMemo *m = memoOf(x, y);
  if (!(m->has & 4)) {
    bool wet = poolNoise(x, y, deep()) < CAVES[int(caveBiomeAt(x, y))].pools &&
               !spared(x, y, 16);
    m = memoOf(x, y);
    m->wet = wet, m->has |= 4;
  }
  return m->wet;
}

bool isWater(int x, int y) {
  return pool(x, y) && CAVES[int(caveBiomeAt(x, y))].pool != CavePool::Lava;
}

bool isRamp(int x, int y) {
  int h = heightAt(x, y), b = (x - (x < 0 ? 2 : 0)) / 3 * 3;
  if (h == 0 || h == ROCK || hash2(b * 13 + 7, y * 29 - 3) % 3 == 2)
    return false;
  for (int c = b; c < b + 3; c++) {
    int below = heightAt(c, y + 1);
    if (heightAt(c, y) != h || below >= h || below == ROCK || heightAt(c, y - 1) < h)
      return false;
  }
  return true;
}

bool structureIn(int, int, Structure &) { return false; }

Sprite groundTile(int x, int y) {
  if (isChasm(x, y))
    return atlas::under(atlas::ug::Void);
  if (isRock(x, y))
    return atlas::tile(0);
  if (isLava(x, y))
    return atlas::carpet(atlas::cp::Lava, 0);
  int r = int(hash2(x * 5 + 1, y * 11 - 3) % 100);
  return CAVES[int(caveBiomeAt(x, y))].floor[r < 80 ? 0 : r < 92 ? 1 : 2];
}

bool propAt(int x, int y, Prop &p) {
  if (edits)
    if (const Edits::Change *c = edits->at(x, y))
      return c->placed && pieceProp(c->piece, x, y, p);
  if (mouthProp(x, y, p))
    return true;
  if (deep()) {
    const DeepCell &d = deepOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
    if (d.biome == CaveBiome::Halls && x == d.hx && y == d.hy - 1) {
      p.fire({1, 0.78f, 0.42f, 1}, 150, -6);
      return true;
    }
  }
  if (crackAt(x, y)) {
    p.add(atlas::under(atlas::ug::Crack), 6);
    p.fire({0.5f, 0.42f, 0.3f, 1}, 40, 2);
    p.kind = "cracked wall";
    return true;
  }
  if (inVault(x, y) && !pool(x, y)) {
    uint32_t v = hash2(x * 17 - 3, y * 23 + 9);
    return v % 100 < 34 && feature::oreVein.place(&feature::oreVein, v >> 16, &p);
  }
  if (isChasm(x, y)) {
    const CaveCell &c = cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
    if (x == c.px && y == c.py)
      p.fire({0.42f, 0.28f, 0.8f, 1}, 110, 4);
    return x == c.px && y == c.py;
  }
  if (isLava(x, y)) {
    if ((x + y * 3) % 5 == 0)
      p.fire({0.6f, 0.24f, 0.06f, 1}, 70, -4);
    return (x + y * 3) % 5 == 0;
  }
  if (isRock(x, y) || pool(x, y) || isRamp(x, y) || isRamp(x, y - 1) || spared(x, y, 9))
    return false;
  uint32_t h = hash2(x * 7 + 3, y * 13 + 1);
  if (hash2(x * 29 + 5, y * 31 - 7) % 1000 < (deep() ? 30u : 14u) &&
      (isRock(x - 1, y) || isRock(x + 1, y) || isRock(x, y - 1)))
    return feature::oreVein.place(&feature::oreVein, h >> 16, &p);
  int r = int((h >> 4) % 1000);
  for (const CaveSpawn &s : CAVES[int(caveBiomeAt(x, y))].spawns) {
    if (r < s.per)
      return s.feature->place(s.feature, h >> 16, &p);
    r -= s.per;
  }
  return false;
}

Color groundTint(int x, int y) {
  if (isLava(x, y))
    return {1, 1, 1, 1};
  if (isChasm(x, y))
    return {1, 1, 1, 1};
  const CaveDef &d = CAVES[int(caveBiomeAt(x, y))];
  return isRock(x, y) ? d.wallTint * 0.25f : d.floorTint;
}

} // namespace under

} // namespace world
