#include "level/Edits.hpp"

#include <spdlog/fmt/fmt.h>

#include <cstdio>

#include "level/Prop.hpp"

namespace tt = atlas::tt;
namespace fr = atlas::fr;
namespace kt = atlas::kt;
namespace t = atlas::t;

namespace {

constexpr int CART = 57;

} // namespace

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
  case Piece::Wall:
    return p.piece(atlas::town(tt::StoneWall), 1.2f, 8);
  case Piece::Bench:
    p.piece(atlas::tile(72), 0.8f, 6);
    p.add(atlas::tile(74), -9);
    return true;
  case Piece::PostEast:
    return p.piece(atlas::town(tt::FenceRight), 0.4f, 6);
  case Piece::CornerNW:
    return p.piece(atlas::town(tt::FenceTopLeft), 0.4f, 6);
  case Piece::CornerNE:
    return p.piece(atlas::town(tt::FenceTopRight), 0.4f, 6);
  case Piece::CornerSW:
    return p.piece(atlas::town(tt::FenceBottomLeft), 0.4f, 6);
  case Piece::CornerSE:
    return p.piece(atlas::town(tt::FenceBottomRight), 0.4f, 6);
  case Piece::EndWest:
    return p.piece(atlas::town(tt::FenceEndLeft), 0.4f, 6);
  case Piece::EndEast:
    return p.piece(atlas::town(tt::FenceEndRight), 0.4f, 6);
  case Piece::Cart:
    return p.piece(atlas::town(CART), 1.0f, 6);
  case Piece::Crates:
    p.piece(atlas::tile(t::Crate), 1.0f, 6);
    p.add(atlas::tile(t::Barrel), -11);
    return true;
  case Piece::Sacks:
    p.piece(atlas::town(tt::Sack), 0.8f, 6);
    p.add(atlas::town(tt::Sack), -7);
    return true;
  case Piece::Logs:
    p.piece(atlas::town(tt::Log), 0.9f, 6);
    p.add(atlas::town(tt::Log), -6);
    return true;
  case Piece::Thorns:
    return p.piece(atlas::kit(kt::Thornbush), 0.6f, 5);
  case Piece::Sign:
    return p.piece(atlas::town(tt::Sign), 0.5f, 3);
  case Piece::Charred:
    p.piece(atlas::kit(kt::CharredTreeLow), 1.2f, 4);
    p.add(atlas::kit(kt::CharredTreeHigh), -16);
    p.lamp({0.24f, 0.08f, 0.02f, 1}, 56, -8);
    return true;
  case Piece::Skull:
    return p.piece(atlas::kit(kt::Skull), 0.3f, 0);
  case Piece::Ribs:
    return p.piece(atlas::frontier(fr::Ribs), 0.4f, 0);
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
