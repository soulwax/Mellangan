#pragma once

#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>

#include <string>
#include <string_view>

#include "Atlas.hpp"
#include "Math.hpp"

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
    float time, hurt, lowHealth;
};
class Screen {
public:
    static constexpr int W = 480, H = 270;

    Vec2 offset;
    float w = 1, h = 1, scale = 1, u = 1;

    bool load(const std::string &base);
    void unload();

    void beginLight(Color ambient);
    void beginWorld(Color clear);
    void beginGui(SDL_Window *window, const PostFx &fx);

    void color(Color c);
    void sprite(const Sprite &s, Vec2 at, Vec2 origin, Vec2 scale = {1, 1}, float rot = 0);
    void center(const Sprite &s, Vec2 at, float scale = 1, float rot = 0);
    void flash(const Sprite &s, Vec2 at, Vec2 origin, Vec2 scale, float rot, float amount, Color c);
    void glow(Vec2 at, float size, Color c);
    void rect(float x, float y, float rw, float rh, Color c);
    void frame(float x, float y, float rw, float rh, float t, Color c);
    void line(Vec2 a, Vec2 b, float width, Color c);
    void number(int n, Vec2 at, Color c);
    void text(std::string_view s, float x, float y, float size, Color c);
    void textCenter(std::string_view s, float cx, float y, float size, Color c);
    static float textWidth(std::string_view s, float size) { return float(s.size()) * 8 * size; }
    bool visible(Vec2 p, float margin) const;

private:
    VK2DTexture atlas = nullptr, silhouette = nullptr, font = nullptr, world = nullptr, light = nullptr;
    VK2DImage silhouetteImg = nullptr;
    VK2DShader post = nullptr;
    Color current;
    int lightBudget = 0;

    void glyphs(std::string_view s, float x, float y, float size);
    bool loadSilhouette(const std::string &path);
};
