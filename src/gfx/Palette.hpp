#pragma once

#include "gfx/Screen.hpp"

namespace palette {
inline bool safe = false;

inline Color warning(float a) {
  return safe ? Color{1, 0.25f, 0.95f, a} : Color{1, 0.18f, 0.12f, a};
}
inline Color hurt() { return safe ? Color{0.95f, 0.45f, 0.05f, 1} : col::red; }
inline Color heal() { return safe ? Color{0.4f, 0.78f, 1, 1} : col::heal; }
inline Color better() { return safe ? Color{0.4f, 0.78f, 1, 1} : Color{0.45f, 0.95f, 0.45f, 1}; }
inline Color worse() { return safe ? Color{0.95f, 0.5f, 0.1f, 1} : Color{1, 0.4f, 0.35f, 1}; }
} // namespace palette
