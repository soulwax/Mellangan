#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

constexpr float PI = 3.14159265358979f;
constexpr float TAU = PI * 2.0f;

struct Vec2 {
    float x = 0, y = 0;

    constexpr Vec2 operator+(Vec2 o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(Vec2 o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator*(float s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(float s) const { return {x / s, y / s}; }
    constexpr Vec2 operator-() const { return {-x, -y}; }
    Vec2 &operator+=(Vec2 o) { x += o.x; y += o.y; return *this; }
    Vec2 &operator-=(Vec2 o) { x -= o.x; y -= o.y; return *this; }
    Vec2 &operator*=(float s) { x *= s; y *= s; return *this; }
};

inline float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
inline float lengthSq(Vec2 v) { return dot(v, v); }
inline float length(Vec2 v) { return std::sqrt(lengthSq(v)); }
inline Vec2 normalized(Vec2 v) {
    float l = length(v);
    return l > 1e-5f ? v / l : Vec2{0, 0};
}
inline Vec2 perp(Vec2 v) { return {-v.y, v.x}; }
inline Vec2 fromAngle(float a) { return {std::cos(a), std::sin(a)}; }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline Vec2 lerp(Vec2 a, Vec2 b, float t) { return a + (b - a) * t; }
inline float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }
// Frame-rate independent exponential approach factor.
inline float damp(float rate, float dt) { return 1.0f - std::exp(-rate * dt); }

struct Rng {
    uint64_t state = 0x9E3779B97F4A7C15ull;

    uint32_t next() {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        return static_cast<uint32_t>(state >> 16);
    }
    float unit() { return (next() & 0xFFFFFF) / float(0x1000000); }
    float range(float lo, float hi) { return lo + (hi - lo) * unit(); }
    int irange(int lo, int hiInclusive) { return lo + int(next() % uint32_t(hiInclusive - lo + 1)); }
    bool chance(float p) { return unit() < p; }
    Vec2 onCircle() { return fromAngle(range(0, TAU)); }
};

inline uint32_t hash2(int x, int y) {
    uint32_t h = uint32_t(x) * 0x8DA6B343u ^ uint32_t(y) * 0xD8163841u;
    h ^= h >> 13;
    h *= 0x5BD1E995u;
    h ^= h >> 15;
    return h;
}
