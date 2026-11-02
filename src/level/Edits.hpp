#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

class Prop;

enum class Piece {
  Campfire,
  Torch,
  Rail,
  Post,
  Wall,
  Bench,
  PostEast,
  CornerNW,
  CornerNE,
  CornerSW,
  CornerSE,
  EndWest,
  EndEast,
  Cart,
  Crates,
  Sacks,
  Logs,
  Thorns,
  Sign,
  Charred,
  Skull,
  Ribs,
  Rubble,
  Count
};

struct Edits {
  struct Change {
    bool placed = false;
    Piece piece = Piece::Campfire;
    float regrow = 0;
  };

  std::unordered_map<uint64_t, Change> changes;

  static uint64_t key(int x, int y) { return uint64_t(uint32_t(x)) << 32 | uint32_t(y); }
  void harvest(int x, int y, float regrow = 0) { changes[key(x, y)] = {false, Piece::Campfire, regrow}; }
  void place(int x, int y, Piece p) { changes[key(x, y)] = {true, p, 0}; }
  void clear(int x, int y) { changes.erase(key(x, y)); }
  const Change *at(int x, int y) const {
    auto it = changes.find(key(x, y));
    return it == changes.end() ? nullptr : &it->second;
  }
  std::string save() const;
  bool load(std::string_view line);
};

namespace world {

extern Edits *edits;
bool pieceProp(Piece piece, int x, int y, Prop &p);

} // namespace world
