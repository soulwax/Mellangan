#include "level/Chart.hpp"

#include <cstring>

#include "entity/Player.hpp"
#include "gfx/Screen.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "meta/Saves.hpp"

namespace {

const char *const TITLES[BIOME_COUNT] = {
    "THE MEADOWS",    "THE GREENWOOD",   "THE AUTUMN WOODS",  "THE HEATH",
    "THE BARROWS",    "THE SNOWFIELDS",  "THE BLOSSOM GROVE", "THE MIRE",
    "THE DESERT",     "THE JUNGLE",      "THE SAVANNA",       "THE ASHLANDS",
    "CRYSTAL FIELDS", "THE FUNGAL WOOD", "THE BADLANDS",      "THE TAIGA",
    "SALT FLATS","FLOODPLAIN","MOONLIT GROVE","SUNKEN RUINS","EMBER FEN",
    "AMBER WOODS","STORM STEPPE","GLASS DESERT","HOLLOW CAVERNS","STARFALL BASIN"};

static_assert(BIOME_COUNT < Chart::WATER);

bool solid(int cx, int cy) {
  int rock = 0;
  for (int y = 0; y < Chart::CELL; y++)
    for (int x = 0; x < Chart::CELL; x++)
      rock += world::isRock(cx * Chart::CELL + x, cy * Chart::CELL + y);
  return rock * 2 > Chart::CELL * Chart::CELL;
}

bool wet(int cx, int cy) {
  int water = 0;
  for (int y = 0; y < Chart::CELL; y++)
    for (int x = 0; x < Chart::CELL; x++)
      water += world::isWater(cx * Chart::CELL + x, cy * Chart::CELL + y);
  return water * 2 >= Chart::CELL * Chart::CELL;
}

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
  if (lastPage && key == lastKey)
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
  BiomeId b = world::biomeUnder(x, y);
  if (world::layer() != world::Layer::Surface || knows(b))
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
        p = std::make_unique<Page>(), lastPage = nullptr;
      uint8_t &c = p->cell[(y & (PAGE - 1)) * PAGE + (x & (PAGE - 1))];
      if (c & NOW)
        continue;
      if (world::layer() != world::Layer::Surface && solid(x, y))
        continue;
      BiomeId b = c ? BiomeId((c & BIOME) - 1)
                    : world::biomeAt(x * CELL + CELL / 2, y * CELL + CELL / 2);
      c = uint8_t((b + 1) | NOW | (wet(x, y) ? WATER : 0)), charted++, cells[b]++;
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

void Chart::chartStars(int rx, int ry) const {
  if (rx != starX || ry != starY)
    starX = rx, starY = ry, starRow = -STAR_REACH, stars.clear();
  if (starRow > STAR_REACH)
    return;
  for (int x = rx - STAR_REACH; x <= rx + STAR_REACH; x++) {
    Structure site;
    if (world::structureIn(x, ry + starRow, site) && site.kind >= StructureKind::AncientTree)
      stars.push_back(site);
  }
  starRow++;
}

void Chart::pin(Screen &s, const ChartView &v, double px, double py, float size,
               Color c) const {
  double scale = v.cell / (16.0 * CELL);
  float h = size * 0.5f;
  float x = float(v.x + v.w * 0.5 + (px * scale - v.cx * v.cell)),
        y = float(v.y + v.h * 0.5 + (py * scale - v.cy * v.cell));
  bool inside = x - h >= v.x && y - h >= v.y && x + h <= v.x + v.w && y + h <= v.y + v.h;
  if (inside)
    return marker(s, v, px, py, size, c);
  x = std::clamp(x, v.x + h + 1, v.x + v.w - h - 1);
  y = std::clamp(y, v.y + h + 1, v.y + v.h - h - 1);
  s.rect(x - h * 0.6f, y - h * 0.6f, size * 0.6f, size * 0.6f, c.alpha(c.a * 0.7f));
}

void Chart::draw(Screen &s, const Level &level, const ChartView &v,
                 float time) const {
  float half = v.w * 0.5f / v.cell, tall = v.h * 0.5f / v.cell;
  int x0 = int(std::floor(v.cx - half)), x1 = int(std::ceil(v.cx + half));
  int y0 = int(std::floor(v.cy - tall)), y1 = int(std::ceil(v.cy + tall));
  float right = v.x + v.w, bottom = v.y + v.h;
  bool below = world::layer() != world::Layer::Surface;
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
        int cc = run / 2;
        BiomeId b = BiomeId((cc & BIOME) - 1);
        Color t = (cc & WATER ? BIOMES[b]->winter() ? Color{0.66f, 0.8f, 0.9f, 1}
                                                    : Color{0.2f, 0.42f, 0.72f, 1}
                   : below    ? CAVES[int(world::caveBiomeAt(from * CELL + CELL / 2, cy * CELL + CELL / 2))].debug * 0.7f
                              : tone(b)) *
                  (run & 1 ? 0.7f : 1.0f);
        if (!(cc & NOW))
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
  const Player &hero = *level.player;
  if (hero.bonus().starChart > 0) {
    int rx = world::regionOf(int(std::floor(hero.x / 16))),
        ry = world::regionOf(int(std::floor(hero.y / 16)));
    chartStars(rx, ry);
    for (const Structure &site : stars)
      pin(s, v, site.centerX(), site.centerY(), dot,
          Color{0.75f, 0.55f, 1, level.visited(site) ? 1.0f : 0.75f});
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
constexpr uint32_t MAGIC = 0x3343414Du;
constexpr uint32_t SCALED_MAGIC = 0x3443414Du;
constexpr uint32_t DISTANCE_MAGIC = 0x3543414Du;
constexpr size_t PAGE_BYTES = 8 + Chart::PAGE * Chart::PAGE;

bool fits(std::string_view s) {
  uint32_t head[3] = {};
  if (s.size() < sizeof head)
    return false;
  std::memcpy(head, s.data(), sizeof head);
  return (head[0] == MAGIC || head[0] == SCALED_MAGIC || head[0] == DISTANCE_MAGIC) &&
         s.size() == sizeof head + size_t(head[1]) * PAGE_BYTES + size_t(head[2]) * sizeof(Structure);
}
} // namespace

void Chart::load(const std::string &path) {
  if (path.empty())
    return;
  std::string file = Saves::recall(path, fits);
  if (!fits(file))
    return;
  const uint8_t *data = reinterpret_cast<const uint8_t *>(file.data());
  uint32_t head[3];
  std::memcpy(head, data, sizeof head);
  const uint8_t *at = data + sizeof head;
  for (uint32_t i = 0; i < head[1]; i++, at += PAGE_BYTES) {
    int32_t px, py;
    std::memcpy(&px, at, 4), std::memcpy(&py, at + 4, 4);
    auto p = std::make_unique<Page>();
    for (int k = 0; k < PAGE * PAGE; k++) {
      p->cell[k] = (at[8 + k] & ~WATER) <= BIOME_COUNT ? at[8 + k] : 0;
      if (p->cell[k] && head[0] != DISTANCE_MAGIC)
        p->cell[k] = uint8_t(world::biomeAt(
            (px * PAGE + k % PAGE) * CELL + CELL / 2,
            (py * PAGE + k / PAGE) * CELL + CELL / 2) + 1);
    }
    pages[keyOf(px, py)] = std::move(p);
  }
  sites.resize(head[2]);
  if (head[2])
    std::memcpy(sites.data(), at, head[2] * sizeof(Structure));
  std::erase_if(sites, [](const Structure &s) { return unsigned(s.kind) >= unsigned(StructureKind::Count); });
  if (head[0] != DISTANCE_MAGIC) {
    size_t count = 0;
    for (const Structure &old : sites) {
      Structure current;
      if (world::structureIn(old.rx, old.ry, current))
        sites[count++] = current;
    }
    sites.resize(count);
  }
  lastPage = nullptr;
}

void Chart::save(const std::string &path) const {
  if (path.empty() || pages.empty())
    return;
  std::vector<uint8_t> out(12 + pages.size() * PAGE_BYTES +
                           sites.size() * sizeof(Structure));
  uint32_t head[3] = {DISTANCE_MAGIC, uint32_t(pages.size()), uint32_t(sites.size())};
  std::memcpy(out.data(), head, sizeof head);
  uint8_t *at = out.data() + sizeof head;
  for (auto &[key, p] : pages) {
    int32_t px = int32_t(key >> 32), py = int32_t(uint32_t(key));
    std::memcpy(at, &px, 4), std::memcpy(at + 4, &py, 4);
    for (int k = 0; k < PAGE * PAGE; k++)
      at[8 + k] = p->cell[k] & (BIOME | WATER);
    at += PAGE_BYTES;
  }
  if (!sites.empty())
    std::memcpy(at, sites.data(), sites.size() * sizeof(Structure));
  Saves::store(path, std::string_view(reinterpret_cast<const char *>(out.data()), out.size()));
}

void Chart::unveil(double x, double y, int radius) {
  int cx = cellOf(x), cy = cellOf(y);
  for (int dy = -radius; dy <= radius; dy += SIGHT)
    for (int dx = -radius; dx <= radius; dx += SIGHT)
      if (dx * dx + dy * dy <= radius * radius)
        reveal(cx + dx, cy + dy), survey(cx + dx, cy + dy);
}
