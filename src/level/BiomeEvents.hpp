#pragma once

#include <array>
#include <cstdint>

class Level;
class Screen;

struct BiomeEventSchedule {
  int biome = -1, epoch = -1;
  float begins = 0, ends = 0, radius = 0;
  double x = 0, y = 0, dx = 0, dy = 0;
};

class BiomeEvents {
public:
  std::array<int, 26> handled;
  BiomeEventSchedule current;
  bool active = false;

  BiomeEvents() { handled.fill(-1); }
  static BiomeEventSchedule schedule(uint32_t seed, int biome, int epoch,
                                     int rx, int ry);
  void tick(Level &level);
  void render(Screen &s, const Level &level) const;

private:
  float nextPulse = 0;
};
