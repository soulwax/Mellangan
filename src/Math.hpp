#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

constexpr float PI = 3.14159265358979f;
constexpr float TAU = PI * 2.0f;
constexpr float DT = 1.0f / 60.0f;

inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }
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
};

inline Rng rng;

inline uint32_t hash2(int x, int y) {
    uint32_t h = uint32_t(x) * 0x8DA6B343u ^ uint32_t(y) * 0xD8163841u;
    h ^= h >> 13;
    h *= 0x5BD1E995u;
    h ^= h >> 15;
    return h;
}
