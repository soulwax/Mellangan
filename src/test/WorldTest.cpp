#undef NDEBUG
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#endif

#include "test/Tests.hpp"

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/Pool.h"
#include "entity/effect/GlaiveThrust.hpp"
#include "entity/effect/IncenseCloud.hpp"
#include "particle/DeathEffect.hpp"
#include "particle/VisualDetail.hpp"
#include "gfx/StatusVisual.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/skill/Skill.hpp"
#include "item/weapon/Recipe.hpp"
#include "item/talent/ExpansionTalents.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/Ability.hpp"
#include "level/Flavour.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/Trait.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/enemy/Renegade.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Bomb.hpp"
#include "entity/effect/Sweep.hpp"
#include "entity/effect/Strike.hpp"
#include "entity/projectile/Slash.hpp"
#include "entity/projectile/ChakramDisc.hpp"
#include "level/Level.hpp"
#include "level/Noise.h"
#include "level/World.hpp"
#include "level/Prop.hpp"
#include "level/Structure.hpp"
#include "item/Upgrade.hpp"
#include "hero/Hero.hpp"
#include "menu/LevelUpMenu.hpp"
#include "particle/Spark.hpp"
#include "particle/TextParticle.hpp"
#include "particle/debris/Debris.hpp"

void testSpawnShore() {
  uint32_t salt = noise_salt;
  int longest = 0;
  for (uint32_t seed = 0; seed < 100; seed++) {
    noise_reseed(seed);
    for (int y = -5; y <= 5; y++)
      for (int x = -5; x <= 5; x++)
        assert(!world::isWater(x, y));
    for (int dir = 0; dir < 4; dir++) {
      int ox = dir == 0 ? 1 : dir == 1 ? -1 : 0, oy = dir == 2 ? 1 : dir == 3 ? -1 : 0;
      for (int a = -30; a <= 30; a++)
        for (int b = -30, run = 0; b <= 30; b++) {
          int x = ox ? a : b, y = ox ? b : a;
          run = !world::isWater(x, y) && world::isWater(x + ox, y + oy) ? run + 1 : 0;
          longest = std::max(longest, run);
        }
    }
  }
  int rims = 0;
  Prop prop;
  for (uint32_t seed = 0; seed < 100; seed++) {
    noise_reseed(seed);
    for (int y = -2; y <= 2; y++)
      for (int x = -2; x <= 2; x++)
        assert(x * x + y * y >= 7 || !world::propAt(x, y, prop));
    int ring = 0;
    for (int i = -4; i <= 4; i++)
      ring += world::propAt(i, -4, prop) + world::propAt(i, 4, prop) +
              (i > -4 && i < 4 ? world::propAt(-4, i, prop) + world::propAt(4, i, prop) : 0);
    rims += ring >= 10;
  }
  int sites = 0, straight = 0;
  for (uint32_t seed = 0; seed < 10; seed++) {
    noise_reseed(seed);
    Structure st{};
    for (int ry = -6; ry < 6; ry++)
      for (int rx = -6; rx < 6; rx++) {
        bool found = false;
        for (int dy = 0; dy < world::REGION && !found; dy += 2)
          for (int dx = 0; dx < world::REGION && !found; dx += 2)
            found = world::structureAt(rx * world::REGION + dx, ry * world::REGION + dy, st, 0);
        if (!found)
          continue;
        if (st.kind == StructureKind::Ruin) {
          int dirt = 0;
          for (int y = st.y; y < st.y + st.h; y++)
            for (int x = st.x; x < st.x + st.w; x++)
              dirt += world::isDirt(x, y);
          assert(dirt > 4 && dirt * 10 < st.w * st.h * 9);
        }
        int wet = 0;
        for (int y = st.y - 4; y < st.y + st.h + 4; y++)
          for (int x = st.x - 4; x < st.x + st.w + 4; x++) {
            bool w = world::isWater(x, y);
            assert(!w || !st.contains(x, y, 1));
            wet += w;
          }
        if (!wet)
          continue;
        sites++;
        int run = 0, best = 0;
        for (int dir = 0; dir < 4; dir++) {
          int ox = dir == 0 ? 1 : dir == 1 ? -1 : 0, oy = dir == 2 ? 1 : dir == 3 ? -1 : 0;
          for (int a = -12; a <= 12; a++) {
            run = 0;
            for (int b = -12; b <= 12; b++) {
              int x = st.x + st.w / 2 + (ox ? a : b), y = st.y + st.h / 2 + (ox ? b : a);
              run = !world::isWater(x, y) && world::isWater(x + ox, y + oy) ? run + 1 : 0;
              best = std::max(best, run);
            }
          }
        }
        straight += best >= 8;
      }
  }
  noise_reseed(salt);
  assert(longest < 18 && rims <= 5 && sites > 20 && straight * 10 < sites * 3);
  std::cout << "[TEST] Shores and clearings stay organic: longest straight spawn coast "
            << longest << ", dense square clearing rims " << rims << "/100, straight structure shores "
            << straight << "/" << sites << ": PASS" << std::endl;
}

void testHills() {
  uint32_t salt = noise_salt;
  int cliffs = 0, straight = 0, peaks = 0, ramps = 0, tarns = 0, land[2]{}, woods[2]{};
  Prop prop;
  for (uint32_t seed = 0; seed < 2; seed++) {
    noise_reseed(seed);
    for (int sx = -180; sx < 180; sx += 120)
      for (int y = -180; y < 180; y++)
        for (int x = sx, run = 0; x < sx + 120; x++) {
          int h = world::heightAt(x, y);
          auto at = [&](int dx, int dy) { return world::heightAt(x + dx, y + dy); };
          for (int k = 1; k <= h; k++)
            assert((at(0, -1) >= k || at(0, 1) >= k) && (at(-1, 0) >= k || at(1, 0) >= k));
          assert(std::abs(h - at(1, 0)) < 2 && std::abs(h - at(0, 1)) < 2);
          if (world::isRamp(x, y)) {
            int b = x - ((x % 3) + 3) % 3;
            assert(world::isRamp(b, y) && world::isRamp(b + 1, y) && world::isRamp(b + 2, y));
            ramps++;
          }
          peaks += h == 3;
          if (world::isWater(x, y)) {
            for (int d = 0; h && d < 9; d++)
              assert(at(d % 3 - 1, d / 3 - 1) == h);
            tarns += h > 0;
          } else {
            land[h >= 2]++;
            woods[h >= 2] += world::propAt(x, y, prop) && prop.solid > 0 && prop.solid < 4;
          }
          bool cliff = h > at(0, 1);
          run = cliff ? run + 1 : 0;
          cliffs += cliff;
          straight += run == 8 ? 8 : run > 8;
        }
  }
  noise_reseed(salt);
  float low = float(woods[0]) / land[0], high = float(woods[1]) / land[1];
  assert(peaks > 0 && ramps > 0 && tarns > 0 && straight * 5 < cliffs && high < low * 0.8f);
  std::cout << "[TEST] Hills rise in clean organic terraces: " << peaks << " hilltop tiles, "
            << ramps << " stair tiles, " << straight << "/" << cliffs
            << " cliff tiles in straight runs, " << tarns << " tarn tiles, woods "
            << int(low * 100) << "% low and " << int(high * 100) << "% high: PASS" << std::endl;
}

void testMasonry() {
  uint32_t salt = noise_salt;
  int seen[5]{}, parts = 0;
  for (int m = 0; m < 4; m++) {
    Sprite s = atlas::masonry(m, 36);
    assert(s.x == (m % 2) * 216 + 1 && s.y == 1488 + (m / 2) * 144 + 1);
    assert(atlas::masonry(m, 131).y + 16 <= 1776);
  }
  auto plainTown = [](Sprite s) { return s.x >= 216 && s.y >= 54 && s.y < 198; };
  for (uint32_t seed = 0; seed < 3; seed++) {
    noise_reseed(seed);
    for (int ry = -16; ry < 16; ry++)
      for (int rx = -16; rx < 16; rx++) {
        Structure st;
        if (!world::structureIn(rx, ry, st))
          continue;
        BiomeId b = world::biomeAt(rx * world::REGION + world::REGION / 2,
                                   ry * world::REGION + world::REGION / 2);
        assert(st.masonry >= 0 && st.masonry <= 4);
        if (b == BIOME_DESERT || b == BIOME_SAVANNA || b == BIOME_BADLANDS)
          assert(st.masonry == 1);
        if (b == BIOME_SNOW || b == BIOME_TAIGA)
          assert(st.masonry == 2);
        if (b == BIOME_ASHLANDS || b == BIOME_EMBER)
          assert(st.masonry == 4);
        if (b == BIOME_MEADOW || b == BIOME_FOREST)
          assert(st.masonry == 0);
        seen[st.masonry]++;
        for (int ly = 0; ly < st.h; ly++)
          for (int lx = 0; lx < st.w; lx++) {
            Prop p;
            Sprite g;
            if (structureGround(&st, lx, ly, &g))
              assert(!st.masonry || !plainTown(g));
            if (!structureProp(&st, lx, ly, &p))
              continue;
            for (int i = 0; i < p.count; i++, parts++) {
              Sprite s = p.parts[i].sprite;
              assert(s.x >= 0 && s.y >= 0 && s.x + s.w <= 432 && s.y + s.h <= 1776);
              assert(!st.masonry || !plainTown(s));
            }
          }
      }
  }
  noise_reseed(salt);
  for (int m = 0; m < 5; m++)
    assert(seen[m] > 0);
  std::cout << "[TEST] Structures take their biome's masonry: plain " << seen[0]
            << ", sandstone " << seen[1] << ", frost " << seen[2] << ", moss "
            << seen[3] << ", ash " << seen[4] << ", " << parts
            << " parts checked: PASS" << std::endl;
}
