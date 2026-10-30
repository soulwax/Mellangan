#pragma once

#include "gfx/Screen.hpp"

class Level;

namespace flourish {

void sigil(Level &level, double x, double y, float r, Color c, int spokes = 8);
void echo(Level &level, double x, double y, Sprite body, Color c);
void streak(Level &level, double x0, double y0, double x1, double y1, int icon,
            Color c, int n);
void scatter(Level &level, double x, double y, int icon, Color c, int n,
             float speed, float gravity = 0);

} // namespace flourish
