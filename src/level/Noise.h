#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t noise_salt, noise_epoch;

void noise_reseed(uint32_t salt);
uint32_t noise_hash(int32_t x, int32_t y, uint32_t seed);
float noise_perlin(float x, float y, uint32_t seed);
float noise_fbm(float x, float y, int octaves, uint32_t seed);
float noise_unit(float v);

#ifdef __cplusplus
}
#endif
