#pragma once

#include "gfx/Screen.hpp"

struct Item;

namespace gui {

void itemTooltip(Screen &s, const Item &item, const Item *worn, const char *hero, float x,
                 float y, int cost = -1);
float materials(Screen &s, const int *owned, const int *cost, float x, float y, float size,
                float alpha);

} // namespace gui
