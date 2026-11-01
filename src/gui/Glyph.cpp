#include "gui/Glyph.hpp"

#include "Math.hpp"

namespace gui {

void glyph(Screen &s, const char *const *rows, int size, float x, float y,
           float cell, Color c, float t, const GlyphInk &ink) {
  Color light{lerpf(c.r, 1, ink.lift), lerpf(c.g, 1, ink.lift),
              lerpf(c.b, 1, ink.lift), c.a};
  Color dim = c * 0.55f, shadow = ink.shadow.alpha(ink.shadow.a * c.a);
  for (int pass = 0; pass < 2; pass++)
    for (int row = 0; row < size; row++)
      for (int col = 0; col < size; col++) {
        char k = rows[row][col];
        if (k == '.')
          continue;
        float px = x + col * cell, py = y + row * cell;
        if (pass == 0) {
          if (cell > 1)
            s.rect(px + cell, py + cell, cell, cell, shadow);
          continue;
        }
        Color paint = k == '+' ? light : k == '-' ? dim : c;
        if (k == '*')
          paint = light.alpha(
              c.a * (ink.twinkleBase +
                     ink.twinkleSwing * std::abs(std::sin(t * ink.twinkleSpeed +
                                                          row * ink.rowPhase + col))));
        s.rect(px, py, cell, cell, paint);
      }
}

} // namespace gui
