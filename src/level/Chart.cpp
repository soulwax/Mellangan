#include "level/Chart.hpp"

#include <cstring>

#include "entity/Player.hpp"
#include "gfx/Screen.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"

namespace {

const char *const TITLES[BIOME_COUNT] = {
    "THE MEADOWS",    "THE GREENWOOD", "THE AUTUMN WOODS", "THE HEATH",
    "THE BARROWS",    "THE SNOWFIELDS", "THE BLOSSOM GROVE", "THE MIRE",
    "THE DESERT",     "THE JUNGLE",    "THE SAVANNA",      "THE ASHLANDS",
    "CRYSTAL FIELDS", "THE FUNGAL WOOD"};

} // namespace

const char *Chart::title(BiomeId b) { return TITLES[b]; }

void Chart::border(Screen &s, const ChartView &v, float u) {
  s.rect(v.x, v.y, v.w, v.h, {0.04f, 0.04f, 0.07f, 0.85f});
  s.bracket(v.x - 2 * u, v.y - 2 * u, v.w + 4 * u, v.h + 4 * u, u);
}

Color Chart::tone(BiomeId b) {
  Color c = BIOMES[b]->debug;
  float grey = 0.3f * c.r + 0.59f * c.g + 0.11f * c.b;
  return {lerpf(c.r, grey, 0.1f) * 0.85f, lerpf(c.g, grey, 0.1f) * 0.85f,
          lerpf(c.b, grey, 0.1f) * 0.85f, 1};
}

const Chart::Page *Chart::page(int px, int py) const {
  uint64_t key = keyOf(px, py);
  if (key == lastKey)
    return lastPage;
  auto it = pages.find(key);
  if (it == pages.end())
    return nullptr;
  lastKey = key, lastPage = it->second.get();
  return lastPage;
}

int Chart::at(int cx, int cy) const {
  const Page *p = page(cx >> 5, cy >> 5);
  return p ? p->cell[(cy & (PAGE - 1)) * PAGE + (cx & (PAGE - 1))] : 0;
}

int Chart::tick(double x, double y) {
  int cx = cellOf(x), cy = cellOf(y);
  if (cx != xLast || cy != yLast)
    reveal(cx, cy), survey(cx, cy), xLast = cx, yLast = cy;
  BiomeId b =
      world::biomeAt(int(std::floor(x / 16)), int(std::floor(y / 16)));
  if (knows(b))
    return -1;
  seen |= 1u << b, discovered++;
  return b;
}

void Chart::reveal(int cx, int cy) {
  float reach = (SIGHT + 0.5f) * (SIGHT + 0.5f);
  for (int dy = -SIGHT; dy <= SIGHT; dy++)
    for (int dx = -SIGHT; dx <= SIGHT; dx++) {
      if (float(dx * dx + dy * dy) > reach)
        continue;
      int x = cx + dx, y = cy + dy;
      std::unique_ptr<Page> &p = pages[keyOf(x >> 5, y >> 5)];
      if (!p)
        p = std::make_unique<Page>(), lastKey = ~0ull;
      uint8_t &c = p->cell[(y & (PAGE - 1)) * PAGE + (x & (PAGE - 1))];
      if (c & NOW)
        continue;
      BiomeId b = c ? BiomeId((c & BIOME) - 1)
                    : world::biomeAt(x * CELL + CELL / 2, y * CELL + CELL / 2);
      c = uint8_t((b + 1) | NOW), charted++, cells[b]++;
    }
}

void Chart::survey(int cx, int cy) {
  int rx = world::regionOf(cx * CELL), ry = world::regionOf(cy * CELL);
  Structure s;
  for (int y = ry - 2; y <= ry + 2; y++)
    for (int x = rx - 2; x <= rx + 2; x++) {
      if (!world::structureIn(x, y, s) ||
          !at(cellOf(s.centerX()), cellOf(s.centerY())))
        continue;
      bool fresh = true;
      for (const Structure &k : sites)
        fresh &= k.rx != s.rx || k.ry != s.ry;
      if (fresh)
        sites.push_back(s);
    }
}

void Chart::marker(Screen &s, const ChartView &v, double px, double py,
                   float size, Color c) const {
  double scale = v.cell / (16.0 * CELL);
  float x = float(v.x + v.w * 0.5 + (px * scale - v.cx * v.cell)),
        y = float(v.y + v.h * 0.5 + (py * scale - v.cy * v.cell));
  float h = size * 0.5f;
  if (x - h < v.x || y - h < v.y || x + h > v.x + v.w || y + h > v.y + v.h)
    return;
  s.rect(x - h - 1, y - h - 1, size + 2, size + 2, {0.05f, 0.04f, 0.08f, c.a});
  s.rect(x - h, y - h, size, size, c);
}

void Chart::draw(Screen &s, const Level &level, const ChartView &v,
                 float time) const {
  float half = v.w * 0.5f / v.cell, tall = v.h * 0.5f / v.cell;
  int x0 = int(std::floor(v.cx - half)), x1 = int(std::ceil(v.cx + half));
  int y0 = int(std::floor(v.cy - tall)), y1 = int(std::ceil(v.cy + tall));
  float right = v.x + v.w, bottom = v.y + v.h;
  for (int cy = y0; cy <= y1; cy++) {
    float top = v.y + v.h * 0.5f + float(cy - v.cy) * v.cell;
    float ya = std::max(top, v.y), yb = std::min(top + v.cell, bottom);
    if (yb <= ya)
      continue;
    int run = 0, from = x0;
    for (int cx = x0; cx <= x1 + 1; cx++) {
      int c = cx <= x1 ? at(cx, cy) : 0, key = 0;
      if (c)
        key = c * 2 + (at(cx + 1, cy) != c || at(cx, cy + 1) != c ||
                       at(cx - 1, cy) != c || at(cx, cy - 1) != c);
      if (key == run)
        continue;
      if (run) {
        float left = v.x + v.w * 0.5f + float(from - v.cx) * v.cell;
        float xa = std::max(left, v.x),
              xb = std::min(left + float(cx - from) * v.cell, right);
        int c = run / 2;
        Color t = tone(BiomeId((c & BIOME) - 1)) * (run & 1 ? 0.7f : 1.0f);
        if (!(c & NOW))
          t = {lerpf(t.r, 0.1f, 0.6f), lerpf(t.g, 0.1f, 0.6f),
               lerpf(t.b, 0.14f, 0.6f), 1};
        if (xb > xa)
          s.rect(xa, ya, xb - xa, yb - ya, t);
      }
      run = key, from = cx;
    }
  }

  float dot = std::max(3.0f, v.cell * 0.9f);
  for (const Structure &k : sites) {
    bool lit = k.kind == StructureKind::Shrine && level.night.isLit(k);
    Color c = lit ? col::gold : STRUCTURES[int(k.kind)].debug;
    marker(s, v, k.centerX(), k.centerY(), dot,
           c.alpha(lit || level.visited(k) ? 1 : 0.55f));
  }
  const Night &n = level.night;
  if (n.hasBeacon) {
    float pulse = 1 + 0.35f * std::sin(time * 6);
    marker(s, v, n.beacon.centerX(), n.beacon.centerY(), dot * 1.4f * pulse,
           col::gold);
  }
  const Player &p = *level.player;
  marker(s, v, p.x, p.y, dot * 1.2f,
         col::white.alpha(0.7f + 0.3f * std::sin(time * 8)));
}

namespace {
constexpr uint32_t MAGIC = 0x3143414Du;
constexpr size_t PAGE_BYTES = 8 + Chart::PAGE * Chart::PAGE;
} // namespace

void Chart::load(const std::string &path) {
  size_t size = 0;
  uint8_t *data =
      path.empty() ? nullptr : static_cast<uint8_t *>(SDL_LoadFile(path.c_str(), &size));
  if (!data)
    return;
  uint32_t head[3] = {};
  if (size >= sizeof head)
    std::memcpy(head, data, sizeof head);
  size_t want = sizeof head + head[1] * PAGE_BYTES + head[2] * sizeof(Structure);
  if (head[0] == MAGIC && size == want) {
    const uint8_t *at = data + sizeof head;
    for (uint32_t i = 0; i < head[1]; i++, at += PAGE_BYTES) {
      int32_t px, py;
      std::memcpy(&px, at, 4), std::memcpy(&py, at + 4, 4);
      auto p = std::make_unique<Page>();
      for (int k = 0; k < PAGE * PAGE; k++)
        p->cell[k] = at[8 + k] <= BIOME_COUNT ? at[8 + k] : 0;
      pages[keyOf(px, py)] = std::move(p);
    }
    sites.resize(head[2]);
    std::memcpy(sites.data(), at, head[2] * sizeof(Structure));
    lastKey = ~0ull;
  }
  SDL_free(data);
}

void Chart::save(const std::string &path) const {
  if (path.empty() || pages.empty())
    return;
  std::vector<uint8_t> out(12 + pages.size() * PAGE_BYTES +
                           sites.size() * sizeof(Structure));
  uint32_t head[3] = {MAGIC, uint32_t(pages.size()), uint32_t(sites.size())};
  std::memcpy(out.data(), head, sizeof head);
  uint8_t *at = out.data() + sizeof head;
  for (auto &[key, p] : pages) {
    int32_t px = int32_t(key >> 32), py = int32_t(uint32_t(key));
    std::memcpy(at, &px, 4), std::memcpy(at + 4, &py, 4);
    for (int k = 0; k < PAGE * PAGE; k++)
      at[8 + k] = p->cell[k] & BIOME;
    at += PAGE_BYTES;
  }
  if (!sites.empty())
    std::memcpy(at, sites.data(), sites.size() * sizeof(Structure));
  std::string tmp = path + ".tmp";
  if (SDL_SaveFile(tmp.c_str(), out.data(), out.size()))
    SDL_RenamePath(tmp.c_str(), path.c_str());
}
