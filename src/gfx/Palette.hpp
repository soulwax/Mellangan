#pragma once

#include "gfx/Screen.hpp"

namespace palette {
enum Mode { Standard, Deuteranopia, Protanopia, Tritanopia, Monochrome, MODES };

struct Scheme {
  Color warning, hurt, heal, better, worse, rarity[6], night;
};

constexpr Color WHITE{1, 1, 1, 1}, GREY{0.72f, 0.72f, 0.74f, 1}, VERMILLION{0.95f, 0.45f, 0.05f, 1},
                SKY{0.4f, 0.78f, 1, 1}, ORANGE{0.95f, 0.5f, 0.1f, 1}, MAGENTA{1, 0.25f, 0.95f, 1};

inline constexpr Scheme SCHEMES[MODES] = {
    {{1, 0.18f, 0.12f, 1}, col::red, col::heal, {0.45f, 0.95f, 0.45f, 1}, {1, 0.4f, 0.35f, 1},
     {WHITE, GREY, {0.45f, 0.55f, 1, 1}, {1, 0.92f, 0.35f, 1}, {0.82f, 0.68f, 0.38f, 1}, {1, 0.62f, 0.2f, 1}},
     {0.72f, 0.45f, 1, 1}},
    {MAGENTA, VERMILLION, SKY, SKY, ORANGE,
     {WHITE, GREY, {0.45f, 0.63f, 1, 1}, {0.91f, 1, 0.45f, 1}, {0.75f, 0.41f, 0.34f, 1}, {1, 0.67f, 0, 1}},
     {0, 0.5f, 1, 1}},
    {MAGENTA, VERMILLION, SKY, SKY, ORANGE,
     {WHITE, GREY, {1, 0.25f, 1, 1}, {0.82f, 1, 0.45f, 1}, {0.88f, 0.75f, 0.09f, 1}, {1, 0.17f, 0, 1}},
     {0.88f, 0.22f, 0.55f, 1}},
    {{1, 0.18f, 0.12f, 1}, {1, 0, 0, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {1, 0, 0, 1},
     {WHITE, GREY, {0.07f, 0.52f, 0.75f, 1}, {0.45f, 1, 1, 1}, {0.4f, 0.88f, 0.4f, 1}, {1, 0, 0, 1}},
     {0.88f, 0.4f, 0.64f, 1}},
    {WHITE, {0.85f, 0.15f, 0.2f, 1}, {1, 1, 0.6f, 1}, {1, 1, 0.6f, 1}, {0.85f, 0.15f, 0.2f, 1},
     {WHITE, GREY, {0, 1, 1, 1}, {1, 0.5f, 0.25f, 1}, {0.8f, 0.4f, 0.88f, 1}, {0.25f, 0.87f, 1, 1}},
     {0.77f, 0.22f, 0.88f, 1}}};

inline int mode = Standard;

inline const Scheme &now() { return SCHEMES[mode]; }
inline Color warning(float a) { return now().warning.alpha(a); }
inline Color hurt() { return now().hurt; }
inline Color heal() { return now().heal; }
inline Color better() { return now().better; }
inline Color worse() { return now().worse; }
} // namespace palette
