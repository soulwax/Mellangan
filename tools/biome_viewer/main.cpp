#include <SDL3/SDL.h>
#include <spdlog/fmt/fmt.h>
#include <stb_image_write.h>

#include <cstdlib>
#include <random>
#include <string_view>
#include <string>
#include <vector>

#include "level/Chart.hpp"
#include "level/Noise.h"
#include "level/World.hpp"

namespace {
constexpr int SIDE = 768, PANEL = 240, BAR = 36, W = SIDE + PANEL, H = SIDE + BAR;
constexpr int STEPS[] = {1, 2, 4, 8, 16, 32};

struct Button {
  SDL_FRect r;
  const char *label;
};

class Viewer {
public:
  Viewer(SDL_Renderer *renderer) : renderer(renderer) {
    map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                            SDL_TEXTUREACCESS_STREAMING, SIDE, SIDE);
    SDL_SetTextureScaleMode(map, SDL_SCALEMODE_NEAREST);
    pixels.assign(size_t(SIDE) * SIDE * 4, 0);
    restart(0);
  }
  ~Viewer() { SDL_DestroyTexture(map); }

  void restart(uint32_t salt) {
    seed = salt;
    noise_reseed(salt);
    row = 0;
    sites.clear();
    std::fill(pixels.begin(), pixels.end(), 0);
    status = salt ? fmt::format("seed {:08X}", salt) : "seed 0 (CLASSIC)";
  }

  void refresh() {
    std::random_device rd;
    restart(rd() | 1);
  }

  void zoom(int dir) {
    zoomAt = std::clamp(zoomAt + dir, 0, int(std::size(STEPS)) - 1);
    restart(seed);
  }

  void step() {
    int step = STEPS[zoomAt];
    for (int n = 0; n < 24 && row < SIDE; n++, row++)
      for (int i = 0; i < SIDE; i++) {
        int x = (i - SIDE / 2) * step, y = (row - SIDE / 2) * step;
        Color c = world::isWater(x, y)
                      ? Color{0.1f, 0.25f, 0.85f, 1}
                      : BIOMES[world::biomeAt(x, y)]->debug *
                            (0.7f + 0.2f * world::heightAt(x, y));
        uint8_t *o = &pixels[(size_t(row) * SIDE + i) * 4];
        o[0] = uint8_t(std::clamp(c.r, 0.0f, 1.0f) * 255);
        o[1] = uint8_t(std::clamp(c.g, 0.0f, 1.0f) * 255);
        o[2] = uint8_t(std::clamp(c.b, 0.0f, 1.0f) * 255);
        o[3] = 255;
      }
    if (row >= SIDE && sites.empty() && step >= 4) {
      int span = SIDE * step / world::REGION / 2 + 1;
      for (int ry = -span; ry <= span; ry++)
        for (int rx = -span; rx <= span; rx++) {
          Structure st;
          if (world::structureIn(rx, ry, st))
            sites.push_back(st);
        }
    }
    SDL_UpdateTexture(map, nullptr, pixels.data(), SIDE * 4);
  }

  bool done() const { return row >= SIDE; }

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
    std::string name = fmt::format("biomes-{:08X}-x{}.png", seed, STEPS[zoomAt]);
    status = SDL_SaveFile(name.c_str(), data.data(), data.size())
                 ? "saved " + name
                 : fmt::format("save failed: {}", SDL_GetError());
  }

  void render(float mx, float my) {
    SDL_SetRenderDrawColor(renderer, 24, 20, 32, 255);
    SDL_RenderClear(renderer);
    SDL_FRect dst{0, BAR, SIDE, SIDE};
    SDL_RenderTexture(renderer, map, nullptr, &dst);
    int step = STEPS[zoomAt];
    if (done() && !sites.empty() && showSites) {
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      for (const Structure &st : sites) {
        float x = (st.x + st.w / 2.0f) / step + SIDE / 2.0f,
              y = (st.y + st.h / 2.0f) / step + SIDE / 2.0f + BAR;
        float r = st.landmark() ? 2.5f : 1.0f;
        SDL_FRect d{x - r, y - r, 2 * r, 2 * r};
        SDL_RenderFillRect(renderer, &d);
      }
    }
    SDL_SetRenderDrawColor(renderer, 255, 240, 200, 255);
    SDL_FRect origin{SIDE / 2.0f - 2, BAR + SIDE / 2.0f - 2, 4, 4};
    SDL_RenderRect(renderer, &origin);
    for (const Button &b : buttons()) {
      SDL_SetRenderDrawColor(renderer, 60, 52, 84, 255);
      SDL_RenderFillRect(renderer, &b.r);
      SDL_SetRenderDrawColor(renderer, 200, 180, 255, 255);
      SDL_RenderRect(renderer, &b.r);
      SDL_RenderDebugText(renderer, b.r.x + 8, b.r.y + 9, b.label);
    }
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    std::string head = fmt::format("{}  |  {} tiles/px, {}x{} tiles  {}", status, step,
                                   SIDE * step, SIDE * step,
                                   done() ? "" : fmt::format("{}%", row * 100 / SIDE));
    SDL_RenderDebugText(renderer, 420, 14, head.c_str());

    float x0 = SIDE + 10;
    SDL_RenderDebugText(renderer, x0, BAR + 6, "BIOMES");
    for (int b = 0; b < BIOME_COUNT; b++) {
      Color c = BIOMES[b]->debug;
      SDL_SetRenderDrawColor(renderer, uint8_t(c.r * 255), uint8_t(c.g * 255),
                             uint8_t(c.b * 255), 255);
      float y = BAR + 22 + b * 15.0f;
      SDL_FRect sw{x0, y, 10, 10};
      SDL_RenderFillRect(renderer, &sw);
      SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
      SDL_RenderDebugText(renderer, x0 + 16, y + 1, Chart::title(BiomeId(b)));
    }
    float ly = BAR + 22 + int(BIOME_COUNT) * 15.0f + 10;
    const char *help[] = {"R/click Refresh: new seed", "0: classic seed",
                          "+/- or wheel: zoom", "C: copy PNG  S: save PNG",
                          "T: toggle structures", "dots: sites, big = landmark"};
    for (const char *h : help)
      SDL_RenderDebugText(renderer, x0, ly, h), ly += 14;
    if (mx >= 0 && mx < SIDE && my >= BAR && my < BAR + SIDE) {
      int x = int(mx - SIDE / 2) * step, y = int(my - BAR - SIDE / 2) * step;
      std::string at = fmt::format("tile {},{}", x, y);
      std::string what = world::isWater(x, y)
                             ? std::string("water")
                             : fmt::format("{} h{}", Chart::title(world::biomeAt(x, y)),
                                           world::heightAt(x, y));
      SDL_RenderDebugText(renderer, x0, H - 34, at.c_str());
      SDL_RenderDebugText(renderer, x0, H - 20, what.c_str());
    }
    SDL_RenderPresent(renderer);
  }

  std::vector<Button> buttons() const {
    return {{{8, 4, 92, 28}, "Refresh"},
            {{108, 4, 92, 28}, "Copy"},
            {{208, 4, 92, 28}, "Save"},
            {{308, 4, 40, 28}, " -"},
            {{354, 4, 40, 28}, " +"}};
  }

  bool showSites = true;

private:
  SDL_Renderer *renderer;
  SDL_Texture *map;
  std::vector<uint8_t> pixels, clip;
  std::vector<Structure> sites;
  std::string status;
  uint32_t seed = 0;
  int row = 0, zoomAt = 3;
};
} // namespace

int main(int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO))
    return 1;
  SDL_Window *window;
  SDL_Renderer *renderer;
  if (!SDL_CreateWindowAndRenderer("Mellangan biome viewer", W, H, 0, &window,
                                   &renderer)) {
    SDL_Log("window: %s", SDL_GetError());
    return 1;
  }
  SDL_SetRenderVSync(renderer, 1);
  {
    Viewer v(renderer);
    for (int i = 1; i + 1 < argc; i++)
      if (std::string_view(argv[i]) == "--seed")
        v.restart(uint32_t(std::strtoul(argv[i + 1], nullptr, 16)));
    for (int i = 1; i + 1 < argc; i++)
      if (std::string_view(argv[i]) == "--png") {
        while (!v.done())
          v.step();
        std::vector<uint8_t> data = v.png();
        bool ok = SDL_SaveFile(argv[i + 1], data.data(), data.size());
        SDL_Log("%s %s", ok ? "wrote" : "failed to write", argv[i + 1]);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return ok ? 0 : 1;
      }
    float mx = -1, my = -1;
    for (bool running = true; running;) {
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
          running = false;
        else if (e.type == SDL_EVENT_MOUSE_MOTION)
          mx = e.motion.x, my = e.motion.y;
        else if (e.type == SDL_EVENT_MOUSE_WHEEL)
          v.zoom(e.wheel.y > 0 ? -1 : 1);
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
          SDL_FPoint p{e.button.x, e.button.y};
          auto bs = v.buttons();
          if (SDL_PointInRectFloat(&p, &bs[0].r))
            v.refresh();
          else if (SDL_PointInRectFloat(&p, &bs[1].r))
            v.copy();
          else if (SDL_PointInRectFloat(&p, &bs[2].r))
            v.save();
          else if (SDL_PointInRectFloat(&p, &bs[3].r))
            v.zoom(1);
          else if (SDL_PointInRectFloat(&p, &bs[4].r))
            v.zoom(-1);
        } else if (e.type == SDL_EVENT_KEY_DOWN) {
          switch (e.key.key) {
          case SDLK_ESCAPE:
            running = false;
            break;
          case SDLK_R:
            v.refresh();
            break;
          case SDLK_0:
            v.restart(0);
            break;
          case SDLK_C:
            v.copy();
            break;
          case SDLK_S:
            v.save();
            break;
          case SDLK_T:
            v.showSites = !v.showSites;
            break;
          case SDLK_EQUALS:
          case SDLK_PLUS:
          case SDLK_KP_PLUS:
            v.zoom(-1);
            break;
          case SDLK_MINUS:
          case SDLK_KP_MINUS:
            v.zoom(1);
            break;
          default:
            break;
          }
        }
      }
      if (!v.done())
        v.step();
      v.render(mx, my);
    }
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
