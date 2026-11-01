#include "gui/Text.hpp"

#include <string>

namespace gui {

float wrapText(Screen &s, std::string_view text, float x, float y, float w,
               float size, float line, Color c, bool draw) {
  std::string row;
  size_t at = 0;
  while (at <= text.size()) {
    size_t end = text.find(' ', at);
    std::string_view word = text.substr(at, end == std::string_view::npos
                                                ? std::string_view::npos
                                                : end - at);
    std::string next = row.empty() ? std::string(word)
                                   : row + " " + std::string(word);
    if (!row.empty() && Screen::textWidth(next, size) > w) {
      if (draw)
        s.text(row, x, y, size, c);
      y += line, row = std::string(word);
    } else {
      row = next;
    }
    if (end == std::string_view::npos)
      break;
    at = end + 1;
  }
  if (!row.empty()) {
    if (draw)
      s.text(row, x, y, size, c);
    y += line;
  }
  return y;
}

} // namespace gui
