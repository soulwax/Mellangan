#pragma once

#include "SpriteObject.hpp"

inline bool showParticle(const SpriteObject &particle, bool reduced) {
  if (!reduced || !particle.cosmeticParticle())
    return true;
  uintptr_t value = reinterpret_cast<uintptr_t>(&particle) >> 4;
  value ^= value >> 7;
  value *= 0x9E3779B1u;
  value ^= value >> 11;
  return (value & 3) == 0;
}
