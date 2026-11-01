#pragma once

#include <string_view>

#include "gfx/Screen.hpp"

namespace gui {

float wrapText(Screen &s, std::string_view text, float x, float y, float w,
               float size, float line, Color c, bool draw = true);

} // namespace gui
