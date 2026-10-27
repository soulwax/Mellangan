#include "gfx/Screen.hpp"

#include <VK2D/Blobs.h>
#include <VK2D/stb_image.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <string>

extern "C" VK2DShader vk2dShaderFrom(const uint8_t *vert, int vertSize,
                                     const uint8_t *frag, int fragSize,
                                     uint32_t uniformSize);

struct PostData {
  float time, hurt, lowHealth;
  uint32_t light;
  float resW, resH, vignette, grain;
};

static VK2DShader loadShader(const std::string &path, uint32_t uniformSize) {
  size_t size = 0;
  void *spv = SDL_LoadFile(path.c_str(), &size);
  if (!spv) {
    spdlog::error("Can't read {}: {}", path, SDL_GetError());
    return nullptr;
  }
  VK2DShader s =
      vk2dShaderFrom(VK2DVertUsershader, sizeof(VK2DVertUsershader),
                     static_cast<const uint8_t *>(spv), int(size), uniformSize);
  SDL_free(spv);
  return s;
}

bool Screen::load(const std::string &base) {
  auto path = [&](const char *rel) { return base + rel; };
  atlas = vk2dTextureLoad(path("res/atlas.png").c_str());
  particles = vk2dTextureLoad(path("res/particles.png").c_str());
  font = vk2dTextureLoad(path("res/font.png").c_str());
  post = loadShader(path("res/shaders/post.spv"), sizeof(PostData));
  loadSilhouette(path("res/atlas.png"));
  world = vk2dTextureCreate(W, H);
  light = vk2dTextureCreate(W, H);
  if (atlas && silhouette && particles && font && post && world && light)
    return true;
  spdlog::error("Failed to load resources from {}res: {}", base,
                vk2dStatusMessage());
  return false;
}

void Screen::unload() {
  if (post)
    vk2dShaderFree(post);
  for (VK2DTexture t : {atlas, silhouette, particles, font, world, light})
    if (t)
      vk2dTextureFree(t);
  if (silhouetteImg)
    vk2dImageFree(silhouetteImg);
}

void Screen::beginLight(Color ambient) {
  vk2dRendererSetTarget(light);
  vk2dRendererSetBlendMode(VK2D_BLEND_MODE_BLEND);
  color(ambient);
  vk2dRendererClear();
  vk2dRendererSetBlendMode(VK2D_BLEND_MODE_ADD);
  lightBudget = 240;
}

void Screen::beginWorld(Color clear) {
  vk2dRendererSetTarget(world);
  vk2dRendererSetBlendMode(VK2D_BLEND_MODE_BLEND);
  color(clear);
  vk2dRendererClear();
  color(col::white);
}

void Screen::beginGui(SDL_Window *window, const PostFx &fx) {
  vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
  int pw, ph;
  SDL_GetWindowSizeInPixels(window, &pw, &ph);
  w = float(std::max(pw, 1)), h = float(std::max(ph, 1));

  VK2DCameraSpec cam{};
  cam.type = VK2D_CAMERA_TYPE_DEFAULT;
  cam.w = w, cam.h = h, cam.zoom = 1;
  vk2dRendererSetCamera(cam);

  float fit = std::min(w / W, h / H);
  scale = fit >= 1 ? std::floor(fit) : fit;
  u = std::max(1.0f, std::round(scale * 0.5f));
  xOffset = yOffset = 0;

  color(col::black);
  vk2dRendererClear();
  color(col::white);

  PostData data{fx.time,  fx.hurt,  fx.lowHealth, vk2dTextureGetID(light),
                float(W), float(H), 0.85f,        0.02f};
  vk2dRendererDrawShader(post, &data, world, std::floor((w - W * scale) / 2),
                         std::floor((h - H * scale) / 2), scale, scale, 0, 0, 0,
                         0, 0, W, H);
}

void Screen::color(Color c) {
  current = c;
  const float v[4] = {c.r, c.g, c.b, c.a};
  vk2dRendererSetColourMod(v);
}

void Screen::particle(int icon, double x, double y, Color c) {
  color(c);
  vk2dRendererDrawTexture(particles, float(x - xOffset - 4),
                          float(y - yOffset - 4), 1, 1, 0, 0, 0,
                          float(icon % 8 * 8), float(icon / 8 * 8), 8, 8);
  color(col::white);
}

void Screen::sprite(const Sprite &s, double x, double y, float xo, float yo,
                    float xs, float ys, float rot) {
  float xp = float(x - xOffset - xo * xs), yp = float(y - yOffset - yo * ys);
  vk2dRendererDrawTexture(atlas, xp, yp, xs, ys, rot, xo, yo, s.x, s.y, s.w,
                          s.h);
}

void Screen::center(const Sprite &s, double x, double y, float sc, float rot) {
  sprite(s, x, y, s.w * 0.5f, s.h * 0.5f, sc, sc, rot);
}

bool Screen::loadSilhouette(const std::string &path) {
  int iw, ih, n;
  stbi_uc *px = stbi_load(path.c_str(), &iw, &ih, &n, 4);
  if (!px)
    return false;
  uint32_t *texel = reinterpret_cast<uint32_t *>(px);
  for (int i = 0; i < iw * ih; i++)
    texel[i] |= 0x00FFFFFFu;
  silhouetteImg =
      vk2dImageFromPixels(vk2dRendererGetDevice(), px, iw, ih, true);
  stbi_image_free(px);
  if (silhouetteImg)
    silhouette = vk2dTextureLoadFromImage(silhouetteImg);
  return silhouette != nullptr;
}

void Screen::flash(const Sprite &s, double x, double y, float xo, float yo,
                   float xs, float ys, float rot, float amount, Color c) {
  sprite(s, x, y, xo, yo, xs, ys, rot);
  if (amount <= 0)
    return;
  Color prev = current;
  color({c.r, c.g, c.b, prev.a * std::min(amount, 1.0f)});
  float xp = float(x - xOffset - xo * xs), yp = float(y - yOffset - yo * ys);
  vk2dRendererDrawTexture(silhouette, xp, yp, xs, ys, rot, xo, yo, s.x, s.y,
                          s.w, s.h);
  color(prev);
}

void Screen::glow(double x, double y, float size, Color c) {
  if (lightBudget <= 0 || !visible(x, y, size * 0.5f))
    return;
  lightBudget--;
  color(c);
  center(atlas::Glow, x, y, size / atlas::Glow.w);
}

void Screen::rect(double x, double y, float rw, float rh, Color c) {
  Color prev = current;
  color(c);
  const Sprite &s = atlas::White;
  vk2dRendererDrawTexture(atlas, float(x - xOffset), float(y - yOffset),
                          rw / s.w, rh / s.h, 0, 0, 0, s.x, s.y, s.w, s.h);
  color(prev);
}

void Screen::frame(double x, double y, float rw, float rh, float t, Color c) {
  rect(x, y, rw, t, c);
  rect(x, y + rh - t, rw, t, c);
  rect(x, y + t, t, rh - 2 * t, c);
  rect(x + rw - t, y + t, t, rh - 2 * t, c);
}

void Screen::bracket(float x, float y, float w, float h, float u) {
  float tick = std::min(8 * u, std::min(w, h) * 0.25f);
  frame(x, y, w, h, u, col::muted.alpha(0.18f));
  Color c = col::muted.alpha(0.8f);
  for (int i = 0; i < 4; i++) {
    rect(i & 1 ? x + w - tick : x, i & 2 ? y + h - u : y, tick, u, c);
    rect(i & 1 ? x + w - u : x, i & 2 ? y + h - tick : y, u, tick, c);
  }
}

void Screen::line(double x0, double y0, double x1, double y1, float width,
                  Color c) {
  double xd = x1 - x0, yd = y1 - y0;
  Color prev = current;
  color(c);
  const Sprite &s = atlas::White;
  sprite(s, x0, y0, 0, s.h * 0.5f, float(std::sqrt(xd * xd + yd * yd) / s.w),
         width / s.h, float(std::atan2(yd, xd)));
  color(prev);
}

void Screen::number(int n, double x, double y, Color c) {
  std::string s = std::to_string(std::max(n, 0));
  double xp = std::round(x - (s.size() * 4 + 1) * 0.5), yp = std::round(y - 3);
  Color prev = current;
  color(c);
  for (char ch : s)
    sprite(atlas::digit(ch - '0'), xp, yp, 0, 0), xp += 4;
  color(prev);
}

void Screen::glyphs(std::string_view s, double x, double y, float size) {
  for (unsigned char ch : s) {
    if (ch != ' ')
      vk2dRendererDrawTexture(font, float(x - xOffset), float(y - yOffset),
                              size, size, 0, 0, 0, float(ch % 16 * 8),
                              float(ch / 16 * 16), 8, 16);
    x += 8 * size;
  }
}

void Screen::text(std::string_view s, double x, double y, float size, Color c) {
  Color prev = current;
  color({0.02f, 0.01f, 0.05f, c.a * 0.85f});
  glyphs(s, x + size, y + size, size);
  color(c);
  glyphs(s, x, y, size);
  color(prev);
}

void Screen::textCenter(std::string_view s, double cx, double y, float size,
                        Color c) {
  text(s, std::floor(cx - textWidth(s, size) * 0.5f), y, size, c);
}

bool Screen::visible(double x, double y, float m) const {
  double xv = x - xOffset, yv = y - yOffset;
  return xv > -m && yv > -m && xv < W + m && yv < H + m;
}
