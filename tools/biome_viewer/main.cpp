#include <SDL3/SDL.h>
#include <spdlog/fmt/fmt.h>
#include <stb_image_write.h>

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <random>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Game.hpp"
#include "level/Chart.hpp"
#include "level/Noise.h"
#include "level/World.hpp"

namespace {
constexpr int SIDE = 768, PANEL = 264, BAR = 36, W = SIDE + PANEL, H = SIDE + BAR, LINE = 11, MAX_CODE = 12;
constexpr int STEPS[] = {1, 2, 4, 8, 16, 32};
constexpr int STEP_COUNT = int(std::size(STEPS));
constexpr int MOUTH_ZOOM = 2, COARSE = 8, TILE = 8, SITE_ZOOM = 3;
constexpr size_t REGION_BATCH = 16;
constexpr uint64_t BUDGET_NS = 12'000'000;

struct LayerDef {
  world::Layer layer;
  const char *name, *file;
  int deepestZoom;
};
constexpr LayerDef LAYERS[] = {{world::Layer::Surface, "SURFACE", "surface", STEP_COUNT - 1},
                               {world::Layer::Caves, "CAVES", "caves", 3},
                               {world::Layer::Deep, "DEEP", "deep", 3}};

constexpr Color WATER{0.1f, 0.25f, 0.85f, 1}, ICE{0.72f, 0.88f, 0.98f, 1}, LAVA{1, 0.42f, 0.08f, 1},
    RAMP{0.95f, 0.72f, 0.38f, 1}, VOID{0.02f, 0.01f, 0.04f, 1}, VAULT_FLOOR{0.78f, 0.66f, 0.3f, 1};
constexpr Color GUARDIAN{0.95f, 0.25f, 0.25f, 1}, MOUTH{1, 0.9f, 0.3f, 1}, CHASM{0.75f, 0.3f, 0.95f, 1},
    VAULT{1, 0.75f, 0.2f, 1}, LANDING{0.4f, 0.9f, 1, 1};
constexpr Color WAYS[] = {{1, 1, 1, 1},          MOUTH,
                          {1, 0.55f, 0.15f, 1},  {0.45f, 0.85f, 0.3f, 1},
                          {1, 0.4f, 0.1f, 1},    {0.3f, 0.55f, 1, 1}};
constexpr const char *WAY_NAMES[] = {"", "cave mouth", "stair", "roots", "chimney", "sinkhole"};
constexpr const char *CAVE_KINDS[] = {"solid", "pocket", "through cave", "warren", "descent"};

struct Key {
  const char *label;
  Color c;
};
const std::vector<Key> CAVE_KEYS = {{"guardian's heart", GUARDIAN}, {"mouth to the surface", MOUTH},
                                    {"chasm to the deep", CHASM},   {"sealed vault", VAULT},
                                    {"stair down", WAYS[int(world::Way::Stair)]}};
const std::vector<Key> DEEP_KEYS = {{"lair / hall", GUARDIAN},
                                    {"landing", LANDING},
                                    {"stair up", WAYS[int(world::Way::Stair)]},
                                    {"roots up", WAYS[int(world::Way::Roots)]},
                                    {"chimney up", WAYS[int(world::Way::Chimney)]},
                                    {"sinkhole to surface", WAYS[int(world::Way::Sinkhole)]}};

struct Mark {
  int x, y, w, h;
  Color c;
  float r;
  std::string label;
};

struct Span {
  int from, to, cell;
};

int floorDiv(int v, int d) { return (v - (v < 0 ? d - 1 : 0)) / d; }

void ink(SDL_Renderer *r, Color c) {
  SDL_SetRenderDrawColor(r, uint8_t(clampf(c.r, 0, 1) * 255), uint8_t(clampf(c.g, 0, 1) * 255),
                         uint8_t(clampf(c.b, 0, 1) * 255), 255);
}

std::string daily() {
  SDL_Time now = 0;
  SDL_DateTime d{};
  SDL_GetCurrentTime(&now);
  SDL_TimeToDateTime(now, &d, true);
  return fmt::format("D{:04}{:02}{:02}", d.year, d.month, d.day);
}

class Viewer;
struct Button {
  SDL_FRect r;
  const char *label;
  void (Viewer::*act)();
};
struct Phase {
  size_t count;
  int stride;
  void (Viewer::*run)(int stride, size_t job);
};

class Viewer {
public:
  explicit Viewer(SDL_Renderer *renderer) : renderer(renderer) {
    map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, SIDE, SIDE);
    SDL_SetTextureScaleMode(map, SDL_SCALEMODE_NEAREST);
    pixels.assign(size_t(SIDE) * SIDE * 4, 0);
    world::openCaves(true);
    reseed("");
  }
  ~Viewer() {
    SDL_DestroyTexture(map);
    world::enter(world::Layer::Surface);
  }

  void reseed(std::string to) {
    code = std::move(to);
    noise_reseed(seedSalt(code));
    restart();
  }

  void restart() {
    world::enter(LAYERS[layerAt].layer);
    zoomAt = std::min(zoomAt, LAYERS[layerAt].deepestZoom);
    step = STEPS[zoomAt];
    cols = spans(vx);
    rows = spans(vy);
    next = 0;
    phases.clear();
    if (surfaceLayer()) {
      for (int stride = COARSE; stride >= 1; stride /= 2)
        phases.push_back({sweepJobs(stride), stride, &Viewer::sweep});
      if (zoomAt <= SITE_ZOOM)
        phases.push_back({(regionCount() + REGION_BATCH - 1) / REGION_BATCH, 0, &Viewer::regions});
      if (zoomAt <= MOUTH_ZOOM)
        phases.push_back({rows.size() * cols.size(), 0, &Viewer::mouths});
    } else {
      phases.push_back({rows.size() * cols.size(), 0, &Viewer::block});
    }
    total = 0;
    for (const Phase &p : phases)
      total += p.count;
    marks.clear();
    std::fill(pixels.begin(), pixels.end(), uint8_t(0));
    status = "";
  }

  void refresh() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::string fresh;
    for (int i = 0; i < 8; i++)
      fresh += CODE_CHARS[rng() % (sizeof CODE_CHARS - 1)];
    reseed(fresh);
  }
  void classic() { reseed(""); }
  void today() { reseed(daily()); }

  void zoomIn() { zoomTo(zoomAt - 1, SIDE / 2, SIDE / 2); }
  void zoomOut() { zoomTo(zoomAt + 1, SIDE / 2, SIDE / 2); }
  void zoomTo(int at, int px, int py) {
    at = std::clamp(at, 0, LAYERS[layerAt].deepestZoom);
    if (at == zoomAt)
      return;
    int tx = tileX(px), ty = tileY(py);
    zoomAt = at;
    vx = tx - (px - SIDE / 2) * STEPS[at];
    vy = ty - (py - SIDE / 2) * STEPS[at];
    restart();
  }
  void zoomBy(int dir, float mx, float my) {
    if (onMap(mx, my))
      zoomTo(zoomAt + dir, int(mx), int(my - BAR));
    else
      zoomTo(zoomAt + dir, SIDE / 2, SIDE / 2);
  }

  void surface() { show(0); }
  void caves() { show(1); }
  void deep() { show(2); }
  void cycle() { show((layerAt + 1) % int(std::size(LAYERS))); }
  void show(int at) {
    if (at == layerAt)
      return;
    layerAt = at;
    restart();
  }

  void pan(int dx, int dy) {
    vx += dx * step;
    vy += dy * step;
    restart();
  }
  void home() {
    vx = vy = 0;
    restart();
  }
  void toggleMarks() { showMarks = !showMarks; }

  void grab(float mx, float my) {
    dragging = onMap(mx, my);
    dragX = mx, dragY = my, shiftX = shiftY = 0;
  }
  void drag(float mx, float my) {
    if (dragging)
      shiftX = mx - dragX, shiftY = my - dragY;
  }
  void release() {
    if (!dragging)
      return;
    dragging = false;
    int dx = int(-shiftX), dy = int(-shiftY);
    shiftX = shiftY = 0;
    if (dx || dy)
      pan(dx, dy);
  }

  void startTyping(SDL_Window *window) {
    typing = true;
    draft.clear();
    SDL_StartTextInput(window);
  }
  void stopTyping(SDL_Window *window, bool apply) {
    typing = false;
    SDL_StopTextInput(window);
    if (apply)
      reseed(draft == "CLASSIC" ? "" : draft);
  }
  void type(const char *text) {
    for (const char *c = text; *c; c++)
      if (std::isalnum(uint8_t(*c)) && int(draft.size()) < MAX_CODE)
        draft += char(std::toupper(uint8_t(*c)));
  }
  void erase() {
    if (!draft.empty())
      draft.pop_back();
  }
  bool isTyping() const { return typing; }

  void work() {
    uint64_t until = SDL_GetTicksNS() + BUDGET_NS;
    while (!done() && SDL_GetTicksNS() < until) {
      size_t job = next++;
      for (const Phase &p : phases) {
        if (job < p.count) {
          (this->*p.run)(p.stride, job);
          break;
        }
        job -= p.count;
      }
    }
    SDL_UpdateTexture(map, nullptr, pixels.data(), SIDE * 4);
  }

  bool done() const { return next >= total; }

  int progress() const { return total ? int(next * 100 / total) : 100; }

  std::vector<uint8_t> png() const {
    std::vector<uint8_t> out;
    stbi_write_png_to_func(
        [](void *ctx, void *data, int size) {
          auto *v = static_cast<std::vector<uint8_t> *>(ctx);
          v->insert(v->end(), (uint8_t *)data, (uint8_t *)data + size);
        },
        &out, SIDE, SIDE, 4, pixels.data(), SIDE * 4);
    return out;
  }

  void copy() {
    if (!done()) {
      status = "still generating";
      return;
    }
    clip = png();
    const char *types[] = {"image/png"};
    bool ok = SDL_SetClipboardData(
        [](void *user, const char *, size_t *size) -> const void * {
          auto *v = static_cast<std::vector<uint8_t> *>(user);
          *size = v->size();
          return v->data();
        },
        nullptr, &clip, types, 1);
    status = ok ? "copied PNG to clipboard" : fmt::format("copy failed: {}", SDL_GetError());
  }

  void save() {
    if (!done()) {
      status = "still generating";
      return;
    }
    std::vector<uint8_t> data = png();
    std::string name = fmt::format("world-{}-{}-x{}.png", seedName(), LAYERS[layerAt].file, step);
    status = SDL_SaveFile(name.c_str(), data.data(), data.size()) ? "saved " + name
                                                                 : fmt::format("save failed: {}", SDL_GetError());
  }

  void render(float mx, float my) {
    SDL_SetRenderDrawColor(renderer, 24, 20, 32, 255);
    SDL_RenderClear(renderer);
    SDL_Rect clipRect{0, BAR, SIDE, SIDE};
    SDL_SetRenderClipRect(renderer, &clipRect);
    SDL_FRect dst{shiftX, BAR + shiftY, SIDE, SIDE};
    SDL_RenderTexture(renderer, map, nullptr, &dst);
    if (showMarks)
      for (const Mark &m : marks)
        drawMark(m);
    ink(renderer, {1, 0.94f, 0.78f, 1});
    SDL_FRect spawn{screenX(0) - 3, screenY(0) - 3, 6, 6};
    SDL_RenderRect(renderer, &spawn);
    SDL_SetRenderClipRect(renderer, nullptr);

    for (const Button &b : buttons()) {
      bool on = (b.act == &Viewer::surface && layerAt == 0) || (b.act == &Viewer::caves && layerAt == 1) ||
                (b.act == &Viewer::deep && layerAt == 2);
      SDL_SetRenderDrawColor(renderer, on ? 110 : 60, on ? 92 : 52, on ? 150 : 84, 255);
      SDL_RenderFillRect(renderer, &b.r);
      SDL_SetRenderDrawColor(renderer, 200, 180, 255, 255);
      SDL_RenderRect(renderer, &b.r);
      SDL_RenderDebugText(renderer, b.r.x + (b.r.w - 8 * std::strlen(b.label)) / 2, b.r.y + 10, b.label);
    }

    float x0 = SIDE + 10, y = 8;
    auto text = [&](const std::string &s, Color c = {0.9f, 0.9f, 0.9f, 1}) {
      ink(renderer, c);
      SDL_RenderDebugText(renderer, x0, y, s.c_str());
      y += LINE;
    };
    text(typing ? fmt::format("seed: {}_", draft) : "seed " + seedName(), {1, 0.86f, 0.5f, 1});
    text(fmt::format("{}  {} tiles/px  {}", LAYERS[layerAt].name, step,
                     done() ? std::string("") : fmt::format("{}%", progress())));
    text(fmt::format("centre {},{}  {}x{} tiles", vx, vy, SIDE * step, SIDE * step), {0.7f, 0.7f, 0.75f, 1});
    y += 6;
    legend(x0, y);
    y += 6;
    const char *help[] = {"R/Refresh new seed  0 classic", "D daily seed  Enter type a code",
                          "1 2 3 or Tab: layer", "+/- wheel zoom, drag/arrows pan",
                          "Home: back to spawn", "C copy  S save  T markers"};
    for (const char *h : help)
      text(h, {0.65f, 0.62f, 0.75f, 1});
    y = float(H - 5 * LINE - 6);
    if (!status.empty())
      text(status, {0.6f, 1, 0.6f, 1});
    else
      y += LINE;
    if (onMap(mx, my))
      for (const std::string &s : hover(mx, my - BAR))
        text(s);
    SDL_RenderPresent(renderer);
  }

  std::vector<Button> buttons() const {
    return {{{8, 4, 78, 28}, "Refresh", &Viewer::refresh}, {{90, 4, 58, 28}, "Copy", &Viewer::copy},
            {{152, 4, 58, 28}, "Save", &Viewer::save},     {{218, 4, 32, 28}, "-", &Viewer::zoomOut},
            {{254, 4, 32, 28}, "+", &Viewer::zoomIn},      {{294, 4, 78, 28}, "Surface", &Viewer::surface},
            {{376, 4, 62, 28}, "Caves", &Viewer::caves},   {{442, 4, 54, 28}, "Deep", &Viewer::deep},
            {{504, 4, 62, 28}, "Daily", &Viewer::today},   {{570, 4, 70, 28}, "Classic", &Viewer::classic}};
  }

  void click(float mx, float my) {
    SDL_FPoint p{mx, my};
    for (const Button &b : buttons())
      if (SDL_PointInRectFloat(&p, &b.r))
        return (this->*b.act)();
    grab(mx, my);
  }

  void place(int x, int y) {
    vx = x, vy = y;
    restart();
  }
  void zoomFor(int tilesPerPixel) {
    for (int i = 0; i < STEP_COUNT; i++)
      if (STEPS[i] <= tilesPerPixel)
        zoomAt = i;
    restart();
  }
  bool layerNamed(std::string_view name) {
    for (int i = 0; i < int(std::size(LAYERS)); i++)
      if (name == LAYERS[i].file)
        return show(i), true;
    return false;
  }

private:
  bool surfaceLayer() const { return layerAt == 0; }
  int tileX(int px) const { return vx + (px - SIDE / 2) * step; }
  int tileY(int py) const { return vy + (py - SIDE / 2) * step; }
  float screenX(double tx) const { return float((tx - vx) / step + SIDE / 2.0 + shiftX); }
  float screenY(double ty) const { return float((ty - vy) / step + SIDE / 2.0 + BAR + shiftY); }
  static bool onMap(float mx, float my) { return mx >= 0 && mx < SIDE && my >= BAR && my < BAR + SIDE; }
  std::string seedName() const { return code.empty() ? "CLASSIC" : code; }

  std::vector<Span> spans(int centre) const {
    std::vector<Span> out;
    for (int i = 0; i < SIDE; i++) {
      int cell = floorDiv(centre + (i - SIDE / 2) * step, world::CAVE_CELL);
      if (out.empty() || out.back().cell != cell)
        out.push_back({i, i, cell});
      out.back().to = i + 1;
    }
    return out;
  }

  Color tone(int x, int y) const {
    if (surfaceLayer()) {
      if (world::isWater(x, y))
        return world::isIce(x, y) ? ICE : WATER;
      if (step <= 2 && world::isRamp(x, y))
        return RAMP;
      int h = world::heightAt(x, y);
      Color c = BIOMES[world::biomeAt(x, y)]->debug * (0.7f + 0.2f * h);
      return world::heightAt(x, y + step) < h ? c * 0.78f : c;
    }
    const CaveDef &d = CAVES[int(world::caveBiomeAt(x, y))];
    if (world::isChasm(x, y))
      return VOID;
    if (world::isRock(x, y))
      return d.debug * 0.2f;
    if (world::isLava(x, y))
      return LAVA;
    if (world::isWater(x, y))
      return world::isIce(x, y) ? ICE : WATER;
    if (step <= 2 && world::isRamp(x, y))
      return RAMP;
    if (layerAt == 1 && world::inVault(x, y))
      return VAULT_FLOOR;
    return d.debug * (0.75f + 0.2f * world::heightAt(x, y));
  }

  void paint(int i, int j, int size, Color c) {
    uint8_t rgba[4] = {uint8_t(clampf(c.r, 0, 1) * 255), uint8_t(clampf(c.g, 0, 1) * 255),
                       uint8_t(clampf(c.b, 0, 1) * 255), 255};
    for (int y = j; y < std::min(SIDE, j + size); y++)
      for (int x = i; x < std::min(SIDE, i + size); x++)
        std::memcpy(&pixels[(size_t(y) * SIDE + x) * 4], rgba, 4);
  }

  static size_t sweepJobs(int stride) {
    size_t tiles = size_t(SIDE / stride + TILE - 1) / TILE;
    return tiles * tiles;
  }

  void sweep(int stride, size_t job) {
    int n = SIDE / stride, tiles = (n + TILE - 1) / TILE, a0 = int(job % tiles) * TILE,
        b0 = int(job / tiles) * TILE;
    for (int b = b0; b < std::min(n, b0 + TILE); b++)
      for (int a = a0; a < std::min(n, a0 + TILE); a++) {
        int i = a * stride, j = b * stride;
        if (stride == COARSE || i % (2 * stride) || j % (2 * stride))
          paint(i, j, stride, tone(tileX(i), tileY(j)));
      }
  }

  void block(int, size_t job) {
    const Span &row = rows[job / cols.size()], &col = cols[job % cols.size()];
    for (int j = row.from; j < row.to; j++)
      for (int i = col.from; i < col.to; i++)
        paint(i, j, 1, tone(tileX(i), tileY(j)));
    cellMarks(col.cell, row.cell);
  }

  void mouths(int, size_t job) { cellMarks(cols[job % cols.size()].cell, rows[job / cols.size()].cell); }

  void mark(int x, int y, Color c, float r, std::string label) {
    marks.push_back({x, y, 1, 1, c, r, std::move(label)});
  }

  void cellMarks(int cx, int cy) {
    if (surfaceLayer()) {
      world::CaveCell c;
      if (world::caveCell(cx, cy, c))
        for (int m = 0; m < c.mouths; m++)
          mark(c.mx[m], c.my[m], MOUTH, 1.5f, fmt::format("cave mouth to {}", CAVES[int(c.biome)].name));
      return;
    }
    if (layerAt == 1) {
      world::CaveCell c;
      if (world::caveCell(cx, cy, c)) {
        const CaveDef &d = CAVES[int(c.biome)];
        mark(c.hx, c.hy, GUARDIAN, 2,
             fmt::format("{} heart of {}", d.guardian ? d.guardian : "quiet", CAVE_KINDS[int(c.kind)]));
        for (int m = 0; m < c.mouths; m++)
          mark(c.mx[m], c.my[m], MOUTH, 1.5f, "mouth to the surface");
        if (c.kind == world::CaveKind::Descent)
          mark(c.px, c.py, CHASM, 2, "chasm into the deep");
        if (c.vault)
          mark(c.vx, c.vy, VAULT, 2, "sealed vault, crack at " + fmt::format("{},{}", c.kx, c.ky));
      }
      world::DeepCell d;
      if (world::deepCell(cx, cy, d) && d.way == world::Way::Stair)
        mark(d.wx, d.wy, WAYS[int(d.way)], 2, "stair down to the deep");
      return;
    }
    world::DeepCell d;
    if (!world::deepCell(cx, cy, d))
      return;
    const CaveDef &def = CAVES[int(d.biome)];
    if (d.lair || step <= 2)
      mark(d.hx, d.hy, GUARDIAN, d.lair ? 2.0f : 1.0f,
           d.lair ? fmt::format("lair of {}", def.guardian ? def.guardian : "a keeper") : "deep hall");
    if (d.landing && step <= 2)
      mark(d.lx, d.ly, LANDING, 1.5f, "landing");
    if (d.way != world::Way::None)
      mark(d.wx, d.wy, WAYS[int(d.way)], 2,
           fmt::format("{} up to the {}", WAY_NAMES[int(d.way)], d.way == world::Way::Sinkhole ? "surface" : "caves"));
  }

  int regionTop() const { return world::regionOf(tileY(0)); }
  int regionLeft() const { return world::regionOf(tileX(0)); }
  int regionWidth() const { return world::regionOf(tileX(SIDE - 1)) - regionLeft() + 1; }
  size_t regionCount() const {
    return size_t(regionWidth()) * size_t(world::regionOf(tileY(SIDE - 1)) - regionTop() + 1);
  }

  void regions(int, size_t job) {
    size_t last = std::min(regionCount(), (job + 1) * REGION_BATCH);
    for (size_t n = job * REGION_BATCH; n < last; n++) {
      int rx = regionLeft() + int(n % regionWidth()), ry = regionTop() + int(n / regionWidth());
      Structure st;
      if (world::structureIn(rx, ry, st))
        marks.push_back({st.x, st.y, st.w, st.h, st.debugColor(), st.landmark() ? 2.0f : 1.0f,
                         fmt::format("{} {}x{}", st.name(), st.w, st.h)});
    }
  }

  void drawMark(const Mark &m) {
    float x = screenX(m.x + m.w / 2.0), y = screenY(m.y + m.h / 2.0);
    ink(renderer, m.c);
    if (m.w > 1 && step <= 2) {
      SDL_FRect box{screenX(m.x), screenY(m.y), float(m.w) / step, float(m.h) / step};
      SDL_RenderRect(renderer, &box);
      return;
    }
    float r = m.r;
    SDL_FRect d{x - r, y - r, 2 * r, 2 * r};
    SDL_RenderFillRect(renderer, &d);
    SDL_SetRenderDrawColor(renderer, 10, 8, 14, 255);
    SDL_FRect edge{d.x - 1, d.y - 1, d.w + 2, d.h + 2};
    SDL_RenderRect(renderer, &edge);
  }

  void legend(float x0, float &y) {
    auto swatch = [&](Color c, const char *label) {
      ink(renderer, c);
      SDL_FRect sw{x0, y, 8, 8};
      SDL_RenderFillRect(renderer, &sw);
      SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
      SDL_RenderDebugText(renderer, x0 + 14, y, label);
      y += LINE;
    };
    if (surfaceLayer()) {
      for (int b = 0; b < BIOME_COUNT; b++)
        swatch(BIOMES[b]->debug, Chart::title(BiomeId(b)));
      y += 4;
      for (int k = 0; k < int(StructureKind::Count); k++) {
        Structure st{StructureKind(k)};
        swatch(st.debugColor(), st.name());
      }
      if (zoomAt > SITE_ZOOM) {
        SDL_SetRenderDrawColor(renderer, 150, 140, 170, 255);
        SDL_RenderDebugText(renderer, x0 + 14, y, "(sites: zoom in to x8)");
        y += LINE;
      }
      swatch(MOUTH, zoomAt <= MOUTH_ZOOM ? "cave mouth" : "cave mouth (zoom in)");
      return;
    }
    for (int b = layerAt == 1 ? 0 : CAVE_SHALLOW; b < (layerAt == 1 ? CAVE_SHALLOW : int(CaveBiome::Count)); b++)
      swatch(CAVES[b].debug, CAVES[b].name);
    y += 4;
    for (const Key &k : layerAt == 1 ? CAVE_KEYS : DEEP_KEYS)
      swatch(k.c, k.label);
  }

  std::vector<std::string> hover(float mx, float my) const {
    for (auto it = marks.rbegin(); showMarks && it != marks.rend(); ++it) {
      float x = screenX(it->x + it->w / 2.0), y = screenY(it->y + it->h / 2.0) - BAR;
      float reach = std::max(4.0f, std::max(it->w, it->h) / 2.0f / step);
      if (std::abs(x - mx) <= reach && std::abs(y - my) <= reach)
        return {it->label, fmt::format("at tile {},{}", it->x, it->y), "", ""};
    }
    int x = tileX(int(mx)), y = tileY(int(my));
    std::vector<std::string> out;
    if (surfaceLayer()) {
      out.push_back(fmt::format("tile {},{}  region {},{}", x, y, world::regionOf(x), world::regionOf(y)));
      out.push_back(Chart::title(world::biomeAt(x, y)));
      std::string what = world::isIce(x, y)     ? "ice"
                         : world::isWater(x, y) ? "water"
                                                : fmt::format("height {}", world::heightAt(x, y));
      if (world::isRamp(x, y))
        what += ", stairs";
      if (world::isDirt(x, y))
        what += ", dirt";
      out.push_back(what);
      Structure st;
      out.push_back(world::structureAt(x, y, st) ? fmt::format("in {}", st.name()) : "");
      return out;
    }
    int cx = floorDiv(x, world::CAVE_CELL), cy = floorDiv(y, world::CAVE_CELL);
    out.push_back(fmt::format("tile {},{}  cell {},{}", x, y, cx, cy));
    out.push_back(CAVES[int(world::caveBiomeAt(x, y))].name);
    out.push_back(world::isChasm(x, y)   ? "chasm"
                  : world::isRock(x, y)  ? "rock"
                  : world::isLava(x, y)  ? "lava"
                  : world::isIce(x, y)   ? "ice"
                  : world::isWater(x, y) ? "water"
                                         : fmt::format("floor, height {}", world::heightAt(x, y)));
    world::CaveCell c;
    out.push_back(layerAt == 1 ? (world::caveCell(cx, cy, c) ? CAVE_KINDS[int(c.kind)] : "solid rock") : "");
    return out;
  }

  SDL_Renderer *renderer;
  SDL_Texture *map;
  std::vector<uint8_t> pixels, clip;
  std::vector<Span> rows, cols;
  std::vector<Mark> marks;
  std::vector<Phase> phases;
  std::string code, draft, status;
  size_t next = 0, total = 0;
  int layerAt = 0, zoomAt = 3, step = 8, vx = 0, vy = 0;
  bool showMarks = true, typing = false, dragging = false;
  float dragX = 0, dragY = 0, shiftX = 0, shiftY = 0;
};

const char *option(int argc, char **argv, std::string_view name) {
  for (int i = 1; i + 1 < argc; i++)
    if (argv[i] == name)
      return argv[i + 1];
  return nullptr;
}
} // namespace

int main(int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO))
    return 1;
  SDL_Window *window;
  SDL_Renderer *renderer;
  if (!SDL_CreateWindowAndRenderer("Mellangan world viewer", W, H, 0, &window, &renderer)) {
    SDL_Log("window: %s", SDL_GetError());
    return 1;
  }
  SDL_SetRenderVSync(renderer, 1);
  int result = 0;
  {
    Viewer v(renderer);
    if (const char *seed = option(argc, argv, "--seed")) {
      std::string code;
      for (const char *c = seed; *c; c++)
        if (std::isalnum(uint8_t(*c)))
          code += char(std::toupper(uint8_t(*c)));
      v.reseed(code == "CLASSIC" ? "" : code);
    }
    if (const char *layer = option(argc, argv, "--layer"); layer && !v.layerNamed(layer))
      SDL_Log("unknown layer %s, use surface, caves or deep", layer);
    if (const char *at = option(argc, argv, "--at")) {
      int x = 0, y = 0;
      if (std::sscanf(at, "%d,%d", &x, &y) == 2)
        v.place(x, y);
    }
    if (const char *zoom = option(argc, argv, "--zoom"))
      v.zoomFor(std::atoi(zoom));
    if (const char *path = option(argc, argv, "--png")) {
      while (!v.done())
        v.work();
      std::vector<uint8_t> data = v.png();
      bool ok = SDL_SaveFile(path, data.data(), data.size());
      SDL_Log("%s %s", ok ? "wrote" : "failed to write", path);
      result = ok ? 0 : 1;
    } else {
      float mx = -1, my = -1;
      for (bool running = true; running;) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
          if (e.type == SDL_EVENT_QUIT)
            running = false;
          else if (e.type == SDL_EVENT_MOUSE_MOTION)
            mx = e.motion.x, my = e.motion.y, v.drag(mx, my);
          else if (e.type == SDL_EVENT_MOUSE_WHEEL)
            v.zoomBy(e.wheel.y > 0 ? -1 : 1, mx, my);
          else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT)
            v.click(e.button.x, e.button.y);
          else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT)
            v.release();
          else if (e.type == SDL_EVENT_TEXT_INPUT && v.isTyping())
            v.type(e.text.text);
          else if (e.type == SDL_EVENT_KEY_DOWN && v.isTyping()) {
            if (e.key.key == SDLK_RETURN || e.key.key == SDLK_KP_ENTER)
              v.stopTyping(window, true);
            else if (e.key.key == SDLK_ESCAPE)
              v.stopTyping(window, false);
            else if (e.key.key == SDLK_BACKSPACE)
              v.erase();
          } else if (e.type == SDL_EVENT_KEY_DOWN) {
            switch (e.key.key) {
            case SDLK_ESCAPE:
              running = false;
              break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
              v.startTyping(window);
              break;
            case SDLK_R:
              v.refresh();
              break;
            case SDLK_0:
              v.classic();
              break;
            case SDLK_D:
              v.today();
              break;
            case SDLK_1:
              v.surface();
              break;
            case SDLK_2:
              v.caves();
              break;
            case SDLK_3:
              v.deep();
              break;
            case SDLK_TAB:
              v.cycle();
              break;
            case SDLK_C:
              v.copy();
              break;
            case SDLK_S:
              v.save();
              break;
            case SDLK_T:
              v.toggleMarks();
              break;
            case SDLK_HOME:
              v.home();
              break;
            case SDLK_LEFT:
              v.pan(-SIDE / 4, 0);
              break;
            case SDLK_RIGHT:
              v.pan(SIDE / 4, 0);
              break;
            case SDLK_UP:
              v.pan(0, -SIDE / 4);
              break;
            case SDLK_DOWN:
              v.pan(0, SIDE / 4);
              break;
            case SDLK_EQUALS:
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
              v.zoomIn();
              break;
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
              v.zoomOut();
              break;
            default:
              break;
            }
          }
        }
        if (!v.done())
          v.work();
        v.render(mx, my);
      }
    }
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return result;
}
