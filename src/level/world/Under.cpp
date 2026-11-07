#include "level/world/Internal.hpp"

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

constexpr int MARGIN = 3, MIN_SIZE = 200, ROCK = 3;

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

bool carved(int x, int y, const Plan &p) {
  for (int r = 0; r < p.rooms; r++) {
    const Room &m = p.room[r];
    float dx = x - m.x, dy = y - m.y, u = dx * m.ca + dy * m.sa, v = dy * m.ca - dx * m.sa;
    float e = (u * u) / (m.ra * m.ra) + (v * v) / (m.rb * m.rb) +
              0.35f * noise_fbm(x * 0.08f + 3.7f, y * 0.08f - 1.9f, 2, 3011);
    if (e < 1 && (dx * dx + dy * dy < 12 ||
                  noise_fbm(x * 0.07f - 12.3f, y * 0.07f + 5.1f, 3, 3001) > 0.1f - 0.6f * (1 - e)))
      return true;
  }
  float wx = x + 5 * noise_fbm(x * 0.06f + 1.3f, y * 0.06f + 7.1f, 2, 3041),
        wy = y + 5 * noise_fbm(x * 0.06f - 5.9f, y * 0.06f + 2.2f, 2, 3043);
  for (int l = 0; l < p.linked; l++) {
    const Room &a = p.room[p.links[l][0]], &b = p.room[p.links[l][1]];
    float ex = b.x - a.x, ey = b.y - a.y, len = ex * ex + ey * ey;
    float t = len > 0 ? clampf(((wx - a.x) * ex + (wy - a.y) * ey) / len, 0, 1) : 0;
    float qx = wx - (a.x + ex * t), qy = wy - (a.y + ey * t);
    float width = 2 + 0.6f * (noise_fbm(x * 0.15f + 9.1f, y * 0.15f - 4.4f, 2, 3045) + 1);
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
  k = {cx, cy, kindOf(h), 0, 0, 0, 0, {}, {}, 0, 0, CaveBiome::Roots};
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
                   carved(x0 + i, y0 + j, shape));
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
}

const Cell &cellOf(int cx, int cy) {
  Cell *c = &cells[(cx & 7) | (cy & 7) << 3];
  if (c->epoch != noise_epoch || c->info.cx != cx || c->info.cy != cy) {
    build(c, cx, cy);
    c->epoch = noise_epoch;
  }
  return *c;
}

bool openAt(int x, int y) {
  int cx = floorDiv(x, CAVE_CELL), cy = floorDiv(y, CAVE_CELL);
  const Cell &c = cellOf(cx, cy);
  return c.open[y - cy * CAVE_CELL] >> (x - cx * CAVE_CELL) & 1;
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
    float n = noise_fbm(x * 0.045f + 7.7f, y * 0.045f - 3.3f, 2, 3021);
    m->raw = uint8_t(n > 0.38f ? 2 : n > 0.12f ? 1 : 0), m->has |= 1;
  }
  return m->raw;
}

bool rawAbove(int x, int y, const void *k) {
  return openAt(x, y) && rawLevel(x, y) >= *static_cast<const int *>(k);
}

bool deep() { return active == Layer::Deep; }

} // namespace

bool caveCell(int cx, int cy, CaveCell &out) {
  out = cellOf(cx, cy).info;
  return out.kind != CaveKind::Solid;
}

bool isRock(int x, int y) { return active == Layer::Caves && !openAt(x, y); }

CaveBiome caveBiomeAt(int x, int y) {
  return cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info.biome;
}

bool isLava(int x, int y) {
  return active == Layer::Caves && under::pool(x, y) &&
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

bool mouthProp(int x, int y, Prop &p) {
  if (!caveMouth(x, y))
    return false;
  p.add(atlas::tile(39));
  if (active != Layer::Surface)
    p.fire({0.36f, 0.34f, 0.28f, 1}, 120, -4);
  return true;
}

namespace under {

BiomeId biomeAt(int x, int y) {
  Surfaced up;
  return world::biomeAt(x, y);
}

GroundBlend groundBlend(int x, int y) { return {{biomeAt(x, y)}, {1}, 1}; }

int heightAt(int x, int y) {
  if (deep())
    return 0;
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
  if (deep() || heightAt(x, y) != 0)
    return false;
  CaveMemo *m = memoOf(x, y);
  if (!(m->has & 4)) {
    CaveCell c = cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
    bool wet = noise_fbm(x * 0.05f - 21.7f, y * 0.05f + 14.2f, 3, 3031) < CAVES[int(c.biome)].pools;
    for (int i = -1; i < c.mouths && wet; i++) {
      int dx = x - (i < 0 ? c.hx : c.mx[i]), dy = y - (i < 0 ? c.hy : c.my[i]);
      wet = dx * dx + dy * dy > 16;
    }
    m = memoOf(x, y);
    m->wet = wet, m->has |= 4;
  }
  return m->wet;
}

bool isWater(int x, int y) {
  return pool(x, y) && CAVES[int(caveBiomeAt(x, y))].pool != CavePool::Lava;
}

bool isRamp(int x, int y) {
  if (deep())
    return false;
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
  if (isLava(x, y)) {
    if ((x + y * 3) % 5 == 0)
      p.fire({0.6f, 0.24f, 0.06f, 1}, 70, -4);
    return (x + y * 3) % 5 == 0;
  }
  if (isRock(x, y) || pool(x, y) || isRamp(x, y) || isRamp(x, y - 1))
    return false;
  const CaveCell &c = cellOf(floorDiv(x, CAVE_CELL), floorDiv(y, CAVE_CELL)).info;
  for (int i = -1; i < c.mouths; i++) {
    int dx = x - (i < 0 ? c.hx : c.mx[i]), dy = y - (i < 0 ? c.hy : c.my[i]);
    if (dx * dx + dy * dy < 10)
      return false;
  }
  uint32_t h = hash2(x * 7 + 3, y * 13 + 1);
  int r = int((h >> 4) % 1000);
  for (const CaveSpawn &s : CAVES[int(c.biome)].spawns) {
    if (r < s.per)
      return s.feature->place(s.feature, h >> 16, &p);
    r -= s.per;
  }
  return false;
}

Color groundTint(int x, int y) {
  if (isLava(x, y))
    return {1, 1, 1, 1};
  const CaveDef &d = CAVES[int(caveBiomeAt(x, y))];
  return isRock(x, y) ? d.wallTint * 0.25f : d.floorTint;
}

} // namespace under

} // namespace world
