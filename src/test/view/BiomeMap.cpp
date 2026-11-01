#undef NDEBUG
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#endif

#include "test/Views.hpp"

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "Game.hpp"
#include "level/Chart.hpp"
#include "level/Noise.h"
#include "level/World.hpp"
#include "entity/Player.hpp"
#include "gfx/EnemyWarning.hpp"
#include "menu/LevelUpMenu.hpp"
#include "menu/ShrineMenu.hpp"
#include "level/ExpansionEncounter.hpp"
#include "item/Upgrade.hpp"
#include "entity/enemy/ai/Melee.hpp"
#include "entity/projectile/Orb.hpp"
#include "particle/Spark.hpp"
#include "menu/TitleMenu.hpp"
#include "menu/ClassMenu.hpp"
#include "entity/pickup/Gem.hpp"

int mapBiomes(const char *path) {
  constexpr int SIDE = 768, STEP = 4;
  std::vector<unsigned char> rgb(SIDE * SIDE * 3);
  for (int j = 0; j < SIDE; j++)
    for (int i = 0; i < SIDE; i++) {
      int x = (i - SIDE / 2) * STEP, y = (j - SIDE / 2) * STEP;
      Color c = world::isWater(x, y) ? Color{0.1f, 0.25f, 0.85f, 1}
                                     : BIOMES[world::biomeAt(x, y)]->debug *
                                           (0.7f + 0.2f * world::heightAt(x, y));
      unsigned char *o = &rgb[(j * SIDE + i) * 3];
      o[0] = (unsigned char)(clampf(c.r, 0, 1) * 255),
      o[1] = (unsigned char)(clampf(c.g, 0, 1) * 255),
      o[2] = (unsigned char)(clampf(c.b, 0, 1) * 255);
    }
  int kinds[int(StructureKind::Count)]{}, sites = 0;
  for (int ry = -40; ry < 40; ry++)
    for (int rx = -40; rx < 40; rx++) {
      Structure st;
      if (world::structureIn(rx, ry, st))
        kinds[int(st.kind)]++, sites++;
    }
  std::printf("%d sites in 6400 regions:", sites);
  for (int k = 0; k < int(StructureKind::Count); k++)
    std::printf(" %s %d", Structure{StructureKind(k)}.name(), kinds[k]);
  std::printf("\n");
  int walks = 64, met = 0, landmarks = 0;
  const double LENGTH = 2000, SIGHT = 14, TILES_PER_SECOND = 74.0 / 16;
  for (int w = 0; w < walks; w++) {
    double a = TAU * w / walks, x = std::cos(a * 7) * 600, y = std::sin(a * 5) * 600;
    std::vector<std::pair<int, int>> seen;
    for (double d = 0; d < LENGTH; d += 4) {
      int px = int(x + std::cos(a) * d), py = int(y + std::sin(a) * d);
      int rx = world::regionOf(px), ry = world::regionOf(py);
      for (int j = ry - 1; j <= ry + 1; j++)
        for (int i = rx - 1; i <= rx + 1; i++) {
          Structure st;
          if (!world::structureIn(i, j, st))
            continue;
          double dx = st.x + st.w / 2.0 - px, dy = st.y + st.h / 2.0 - py;
          if (dx * dx + dy * dy > SIGHT * SIGHT ||
              std::find(seen.begin(), seen.end(), std::make_pair(i, j)) != seen.end())
            continue;
          seen.push_back({i, j});
          met++, landmarks += st.landmark();
        }
    }
  }
  double seconds = LENGTH * walks / std::max(1, met) / TILES_PER_SECOND;
  std::printf("pacing: %d sites (%d landmarks) on %d straight walks of %.0f tiles, "
              "one every %.1f s\n",
              met, landmarks, walks, LENGTH, seconds);
  FILE *f = std::fopen(path, "wb");
  if (!f)
    return 1;
  std::fprintf(f, "P6 %d %d 255\n", SIDE, SIDE);
  std::fwrite(rgb.data(), 1, rgb.size(), f);
  std::fclose(f);
  return 0;
}
