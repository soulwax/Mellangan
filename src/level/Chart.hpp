#pragma once

#include <climits>
#include <cmath>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "level/Structure.hpp"
#include "level/biome/Biome.hpp"

class Level;
class Screen;

struct ChartView {
  float x, y, w, h, cell;
  double cx, cy;
};

class Chart {
public:
  static constexpr int CELL = 4, PAGE = 32, SIGHT = 6;

  int charted = 0, discovered = 0;
  int cells[BIOME_COUNT]{};

  int tick(double x, double y);
  int at(int cx, int cy) const;
  bool knows(BiomeId b) const { return seen >> b & 1; }
  void draw(Screen &s, const Level &level, const ChartView &v,
            float time) const;

  static int cellOf(double px) { return int(std::floor(px / (16.0 * CELL))); }
  static void border(Screen &s, const ChartView &v, float u);
  static const char *title(BiomeId b);
  static Color tone(BiomeId b);

private:
  struct Page {
    uint8_t cell[PAGE * PAGE];
  };
  std::unordered_map<uint64_t, std::unique_ptr<Page>> pages;
  std::vector<Structure> sites;
  mutable const Page *lastPage = nullptr;
  mutable uint64_t lastKey = ~0ull;
  uint32_t seen = 0;
  int xLast = INT32_MIN, yLast = INT32_MIN;

  static uint64_t keyOf(int px, int py) {
    return uint64_t(uint32_t(px)) << 32 | uint32_t(py);
  }
  const Page *page(int px, int py) const;
  void reveal(int cx, int cy);
  void survey(int cx, int cy);
  void marker(Screen &s, const ChartView &v, double px, double py, float size,
              Color c) const;
};
