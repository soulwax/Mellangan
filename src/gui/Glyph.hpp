#pragma once

#include "gfx/Screen.hpp"

namespace gui {

struct GlyphInk {
  float lift;
  Color shadow;
  float twinkleBase, twinkleSwing, twinkleSpeed, rowPhase;
};

void glyph(Screen &s, const char *const *rows, int size, float x, float y,
           float cell, Color c, float t, const GlyphInk &ink);

} // namespace gui
