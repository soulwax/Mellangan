#pragma once

#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>

#include <string>
#include <string_view>

#include "Atlas.hpp"

struct Color {
  float r = 1, g = 1, b = 1, a = 1;

  constexpr Color alpha(float v) const { return {r, g, b, v}; }
  constexpr Color operator*(float s) const { return {r * s, g * s, b * s, a}; }
};

namespace col {
constexpr Color white{1, 1, 1, 1};
constexpr Color black{0, 0, 0, 1};
constexpr Color gold{1, 0.82f, 0.32f, 1};
constexpr Color red{0.92f, 0.22f, 0.25f, 1};
constexpr Color heal{0.4f, 1, 0.5f, 1};
constexpr Color xp{0.32f, 0.62f, 1, 1};
constexpr Color panel{0.07f, 0.06f, 0.11f, 0.92f};
constexpr Color muted{0.66f, 0.64f, 0.74f, 1};
} // namespace col

struct PostFx {
  float time, hurt, lowHealth, pause = 0, shrine = 0;
};
class Screen {
public:
  static constexpr int W = 480, H = 270, MAX_W = 640, MAX_H = 360;

  int xOffset = 0, yOffset = 0, vw = W, vh = H;
  float w = 1, h = 1, scale = 1, u = 1;

  ~Screen() { unload(); }
  bool load(const std::string &base);
  void unload();

  void fit(SDL_Window *window);
  float reach() const;
  void beginLight(Color ambient);
  void beginWorld(Color clear);
  void beginGui(SDL_Window *window, const PostFx &fx);
  void targetGui(VK2DTexture target) { screenTarget = target; }
  bool saveTarget(VK2DTexture target, const std::string &path);

  void color(Color c);
  void mirror(double axis, Color veil) { mirrorY = axis, this->veil = veil, mirrored = true; }
  void unmirror() { mirrored = false; }
  void sprite(const Sprite &s, double x, double y, float xo, float yo,
              float xs = 1, float ys = 1, float rot = 0);
  void center(const Sprite &s, double x, double y, float scale = 1,
              float rot = 0);
  void flash(const Sprite &s, double x, double y, float xo, float yo, float xs,
             float ys, float rot, float amount, Color c);
  void glow(double x, double y, float size, Color c);
  void particle(int icon, double x, double y, Color c, float scale = 1,
                float rot = 0);
  void sky(const Sprite &s, double x, double y, Color c, float xs = 1,
           float ys = 1, float rot = 0);
  void rect(double x, double y, float rw, float rh, Color c);
  void frame(double x, double y, float rw, float rh, float t, Color c);
  void bracket(float x, float y, float w, float h, float u);
  void line(double x0, double y0, double x1, double y1, float width, Color c);
  void ring(double x, double y, float r, Color c, float width = 1);
  void ring(double x, double y, float r) { ring(x, y, r, current); }
  void number(int n, double x, double y, Color c);
  void text(std::string_view s, double x, double y, float size, Color c);
  void textCenter(std::string_view s, double cx, double y, float size, Color c);
  static float textWidth(std::string_view s, float size) {
    return float(s.size()) * 8 * size;
  }
  bool visible(double x, double y, float margin) const;

private:
  VK2DTexture atlas = nullptr, silhouette = nullptr, particles = nullptr,
              weather = nullptr, font = nullptr, world = nullptr, light = nullptr, screenTarget = nullptr;
  VK2DImage silhouetteImg = nullptr;
  VK2DShader post = nullptr;
  Color current, veil;
  double mirrorY = 0;
  bool mirrored = false;
  int lightBudget = 0;

  void glyphs(std::string_view s, double x, double y, float size);
  bool loadSilhouette(const std::string &path);
};
