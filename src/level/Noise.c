#include "level/Noise.h"

#include <math.h>

uint32_t noise_hash(int32_t x, int32_t y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 0x8DA6B343u ^ (uint32_t)y * 0xD8163841u ^ seed * 0xCB1AB31Fu;
    h ^= h >> 13;
    h *= 0x5BD1E995u;
    h ^= h >> 15;
    return h;
}

static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }


static const float GRAD[8][2] = {
    {1, 1}, {-1, 1}, {1, -1}, {-1, -1}, {1.41421356f, 0}, {-1.41421356f, 0}, {0, 1.41421356f}, {0, -1.41421356f},
};

static float grad(int32_t ix, int32_t iy, uint32_t seed, float dx, float dy) {
    const float *g = GRAD[noise_hash(ix, iy, seed) >> 29];
    return g[0] * dx + g[1] * dy;
}


static int32_t ifloor(float v) {
    int32_t i = (int32_t)v;
    return i - (v < (float)i);
}

float noise_perlin(float x, float y, uint32_t seed) {
    int32_t ix = ifloor(x), iy = ifloor(y);
    float dx = x - (float)ix, dy = y - (float)iy, u = fade(dx), v = fade(dy);
    float a = grad(ix, iy, seed, dx, dy), b = grad(ix + 1, iy, seed, dx - 1, dy);
    float c = grad(ix, iy + 1, seed, dx, dy - 1), d = grad(ix + 1, iy + 1, seed, dx - 1, dy - 1);
    float ab = a + (b - a) * u, cd = c + (d - c) * u;
    return ab + (cd - ab) * v;
}

float noise_fbm(float x, float y, int octaves, uint32_t seed) {
    float sum = 0, amp = 1, norm = 0;
    for (int i = 0; i < octaves; i++) {
        sum += noise_perlin(x, y, seed + (uint32_t)i * 0x9E3779B9u) * amp;
        norm += amp;
        amp *= 0.5f;
        float nx = x * 1.6f - y * 1.2f, ny = x * 1.2f + y * 1.6f;
        x = nx, y = ny;
    }
    return sum / norm;
}

float noise_unit(float v) {
    v = v * 0.5f + 0.5f;
    return v < 0 ? 0 : v > 1 ? 1 : v;
}
