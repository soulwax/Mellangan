#include "gfx/Screen.hpp"

#include <VK2D/Blobs.h>
#include <VK2D/stb_image.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <string>


extern "C" VK2DShader vk2dShaderFrom(const uint8_t *vert, int vertSize, const uint8_t *frag, int fragSize, uint32_t uniformSize);

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
    VK2DShader s = vk2dShaderFrom(VK2DVertUsershader, sizeof(VK2DVertUsershader), static_cast<const uint8_t *>(spv),
                                  int(size), uniformSize);
    SDL_free(spv);
    return s;
}

bool Screen::load(const std::string &base) {
    auto path = [&](const char *rel) { return base + rel; };
    atlas = vk2dTextureLoad(path("res/atlas.png").c_str());
    font = vk2dTextureLoad(path("res/font.png").c_str());
    post = loadShader(path("res/shaders/post.spv"), sizeof(PostData));
    loadSilhouette(path("res/atlas.png"));
    world = vk2dTextureCreate(W, H);
    light = vk2dTextureCreate(W, H);
    if (atlas && silhouette && font && post && world && light) return true;
    spdlog::error("Failed to load resources from {}res: {}", base, vk2dStatusMessage());
    return false;
}

void Screen::unload() {
    if (post) vk2dShaderFree(post);
    for (VK2DTexture t : {atlas, silhouette, font, world, light})
        if (t) vk2dTextureFree(t);
    if (silhouetteImg) vk2dImageFree(silhouetteImg);
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
    offset = {};

    color(col::black);
    vk2dRendererClear();
    color(col::white);

    PostData data{fx.time, fx.hurt, fx.lowHealth, vk2dTextureGetID(light), float(W), float(H), 0.85f, 0.02f};
    vk2dRendererDrawShader(post, &data, world, std::floor((w - W * scale) / 2), std::floor((h - H * scale) / 2),
                           scale, scale, 0, 0, 0, 0, 0, W, H);
}

void Screen::color(Color c) {
    current = c;
    const float v[4] = {c.r, c.g, c.b, c.a};
    vk2dRendererSetColourMod(v);
}

void Screen::sprite(const Sprite &s, Vec2 at, Vec2 o, Vec2 sc, float rot) {
    Vec2 p = at - offset - Vec2{o.x * sc.x, o.y * sc.y};
    vk2dRendererDrawTexture(atlas, p.x, p.y, sc.x, sc.y, rot, o.x, o.y, s.x, s.y, s.w, s.h);
}

void Screen::center(const Sprite &s, Vec2 at, float sc, float rot) {
    sprite(s, at, {s.w * 0.5f, s.h * 0.5f}, {sc, sc}, rot);
}

bool Screen::loadSilhouette(const std::string &path) {
    int w, h, n;
    stbi_uc *px = stbi_load(path.c_str(), &w, &h, &n, 4);
    if (!px) return false;
    uint32_t *texel = reinterpret_cast<uint32_t *>(px);
    for (int i = 0; i < w * h; i++) texel[i] |= 0x00FFFFFFu; // RGBA bytes, little-endian: set R, G, B, keep A
    silhouetteImg = vk2dImageFromPixels(vk2dRendererGetDevice(), px, w, h, true);
    stbi_image_free(px);
    if (silhouetteImg) silhouette = vk2dTextureLoadFromImage(silhouetteImg);
    return silhouette != nullptr;
}

void Screen::flash(const Sprite &s, Vec2 at, Vec2 o, Vec2 sc, float rot, float amount, Color c) {
    sprite(s, at, o, sc, rot);
    if (amount <= 0) return;
    Color prev = current;
    color({c.r, c.g, c.b, prev.a * std::min(amount, 1.0f)});
    Vec2 p = at - offset - Vec2{o.x * sc.x, o.y * sc.y};
    vk2dRendererDrawTexture(silhouette, p.x, p.y, sc.x, sc.y, rot, o.x, o.y, s.x, s.y, s.w, s.h);
    color(prev);
}

void Screen::glow(Vec2 at, float size, Color c) {
    if (lightBudget <= 0 || !visible(at, size * 0.5f)) return;
    lightBudget--;
    color(c);
    center(atlas::Glow, at, size / atlas::Glow.w);
}

void Screen::rect(float x, float y, float rw, float rh, Color c) {
    Color prev = current;
    color(c);
    const Sprite &s = atlas::White;
    vk2dRendererDrawTexture(atlas, x - offset.x, y - offset.y, rw / s.w, rh / s.h, 0, 0, 0, s.x, s.y, s.w, s.h);
    color(prev);
}

void Screen::frame(float x, float y, float rw, float rh, float t, Color c) {
    rect(x, y, rw, t, c);
    rect(x, y + rh - t, rw, t, c);
    rect(x, y + t, t, rh - 2 * t, c);
    rect(x + rw - t, y + t, t, rh - 2 * t, c);
}

void Screen::line(Vec2 a, Vec2 b, float width, Color c) {
    Vec2 d = b - a;
    Color prev = current;
    color(c);
    const Sprite &s = atlas::White;
    sprite(s, a, {0, s.h * 0.5f}, {length(d) / s.w, width / s.h}, std::atan2(d.y, d.x));
    color(prev);
}

void Screen::number(int n, Vec2 at, Color c) {
    std::string s = std::to_string(std::max(n, 0));
    float x = std::round(at.x - (s.size() * 4 + 1) * 0.5f), y = std::round(at.y - 3);
    Color prev = current;
    color(c);
    for (char ch : s) sprite(atlas::digit(ch - '0'), {x, y}, {0, 0}), x += 4;
    color(prev);
}

void Screen::glyphs(std::string_view s, float x, float y, float size) {
    for (unsigned char ch : s) {
        if (ch != ' ')
            vk2dRendererDrawTexture(font, x - offset.x, y - offset.y, size, size, 0, 0, 0,
                                    float(ch % 16 * 8), float(ch / 16 * 16), 8, 16);
        x += 8 * size;
    }
}

void Screen::text(std::string_view s, float x, float y, float size, Color c) {
    Color prev = current;
    color({0.02f, 0.01f, 0.05f, c.a * 0.85f});
    glyphs(s, x + size, y + size, size);
    color(c);
    glyphs(s, x, y, size);
    color(prev);
}

void Screen::textCenter(std::string_view s, float cx, float y, float size, Color c) {
    text(s, std::floor(cx - textWidth(s, size) * 0.5f), y, size, c);
}

bool Screen::visible(Vec2 p, float m) const {
    Vec2 v = p - offset;
    return v.x > -m && v.y > -m && v.x < W + m && v.y < H + m;
}
