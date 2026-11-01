#pragma once

#include "gfx/Screen.hpp"

struct Item;

namespace gui {

void itemTooltip(Screen &s, const Item &item, const Item *worn, const char *hero, float x,
                 float y);

} // namespace gui
