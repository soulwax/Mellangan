#include "level/Edits.hpp"

#include <spdlog/fmt/fmt.h>

#include <cstdio>

#include "level/Prop.hpp"

namespace tt = atlas::tt;
namespace fr = atlas::fr;

namespace world {

Edits *edits = nullptr;

bool pieceProp(Piece piece, int x, int y, Prop &p) {
  p = Prop{};
  p.x = x * 16 + 8.0, p.y = y * 16 + 16.0;
  switch (piece) {
  case Piece::Campfire:
    p.piece(atlas::town(tt::Log), 0.9f, 5);
    p.fire({0.55f, 0.27f, 0.07f, 1}, 150, -6);
    return true;
  case Piece::Torch:
    p.piece(atlas::frontier(fr::LanternPost), 0.6f, 3);
    p.fire({0.5f, 0.28f, 0.07f, 1}, 90, -6);
    return true;
  case Piece::Rail:
    return p.piece(atlas::town(tt::FenceRail), 0.4f, 6);
  case Piece::Post:
    return p.piece(atlas::town(tt::FenceLeft), 0.4f, 6);
  default:
    return false;
  }
}

} // namespace world

std::string Edits::save() const {
  std::string out;
  for (const auto &[k, c] : changes) {
    int x = int(int32_t(k >> 32)), y = int(int32_t(k & 0xFFFFFFFFu));
    out += c.placed ? fmt::format("edit p {} {} {}\n", x, y, int(c.piece))
                    : fmt::format("edit h {} {} {:.1f}\n", x, y, c.regrow);
  }
  return out;
}

bool Edits::load(std::string_view line) {
  char kind = 0;
  int x = 0, y = 0;
  float v = 0;
  std::string s(line);
  if (std::sscanf(s.c_str(), "edit %c %d %d %f", &kind, &x, &y, &v) != 4)
    return false;
  if (kind == 'p' && v >= 0 && v < float(Piece::Count))
    place(x, y, Piece(int(v)));
  else if (kind == 'h')
    harvest(x, y, v);
  else
    return false;
  return true;
}
