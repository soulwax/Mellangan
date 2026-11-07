#undef NDEBUG
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#endif

#include "test/Tests.hpp"

#include <stdlib.h>
#include <cstring>
#include <filesystem>
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
#include "level/Chart.hpp"
#include "campaign/Campaign.hpp"
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
  for (int m = 0; m < 4; m++)
    assert(atlas::masonryProp(m, 4).x + 16 < 432 && atlas::masonryProp(m, 0).y + 16 <= 1794);
  auto plainTown = [](Sprite s) {
    if (s.x >= 216 && s.y >= 54 && s.y < 198)
      return true;
    for (int i : {atlas::t::Pillar, atlas::t::Crate, atlas::t::GraveCross,
                  atlas::t::GraveStone, atlas::t::Barrel})
      if (s.x == atlas::tile(i).x && s.y == atlas::tile(i).y)
        return true;
    return false;
  };
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
              assert(s.x >= 0 && s.y >= 0 && s.x + s.w <= 432 && s.y + s.h <= 1794);
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

namespace {

uint64_t worldPrint(int x0, int y0, int span) {
  uint64_t h = 1469598103934665603ull;
  auto mix = [&](uint64_t v) { h = (h ^ v) * 1099511628211ull; };
  Prop p;
  for (int y = y0; y < y0 + span; y++)
    for (int x = x0; x < x0 + span; x++) {
      Sprite g = world::groundTile(x, y);
      mix(uint64_t(world::biomeAt(x, y)) | uint64_t(world::heightAt(x, y)) << 8 |
          uint64_t(world::isWater(x, y)) << 12 | uint64_t(world::isDirt(x, y)) << 13 |
          uint64_t(world::isRamp(x, y)) << 14);
      mix(uint64_t(g.x) << 16 | uint64_t(g.y));
      if (world::propAt(x, y, p))
        for (int i = 0; i < p.count; i++)
          mix(uint64_t(p.parts[i].sprite.x) << 20 ^ uint64_t(p.parts[i].sprite.y) << 4 ^
              uint64_t(p.parts[i].yo + 64));
    }
  return h;
}

} // namespace

void testLayers() {
  uint32_t salt = noise_salt;
  world::openCaves(false);
  noise_reseed(7);
  assert(world::layer() == world::Layer::Surface);
  uint64_t before = worldPrint(-96, -64, 160);
  std::vector<BiomeId> above;
  for (int y = -64; y < 64; y += 8)
    for (int x = -96; x < 64; x += 8)
      above.push_back(world::biomeAt(x, y));
  world::enter(world::Layer::Caves);
  assert(world::layer() == world::Layer::Caves);
  size_t i = 0;
  Structure st;
  Prop p;
  for (int y = -64; y < 64; y += 8)
    for (int x = -96; x < 64; x += 8, i++) {
      assert(world::biomeAt(x, y) == above[i]);
      int h = world::heightAt(x, y);
      assert(h >= 0 && h <= 3 && (h == 3) == world::isRock(x, y));
      assert(!world::structureAt(x, y, st, 2) && !world::propAt(x, y, p));
      world::GroundBlend g = world::groundBlend(x, y);
      assert(g.count == 1 && g.biome[0] == above[i] && g.weight[0] == 1);
    }
  assert(!world::nearestStructure(0, 0, 3, st));
  uint64_t below = worldPrint(-96, -64, 160);
  world::enter(world::Layer::Surface);
  uint64_t after = worldPrint(-96, -64, 160);
  world::enter(world::Layer::Caves);
  world::enter(world::Layer::Surface);
  uint64_t again = worldPrint(-96, -64, 160);
  noise_reseed(salt);
  assert(before == after && after == again && below != before);
  std::cout << "[TEST] World layers: surface print " << std::hex << before << std::dec
            << " survives two trips below; the caves layer echoes the biome above: PASS"
            << std::endl;
}

void testLayerSaves() {
  Campaign c;
  c.world = "TESTSEED";
  c.layer = 1;
  c.edits.place(3, 4, Piece::Torch);
  c.below[0].place(-7, 9, Piece::Campfire);
  c.below[0].harvest(5, -2, 30);
  c.below[1].place(100, -200, Piece::Wall);
  std::string text = c.save();
  assert(text.find("cave p -7 9") != std::string::npos && text.find("deep p 100 -200") != std::string::npos);
  Campaign back;
  assert(Campaign::load(text, back));
  assert(back.layer == 1 && back.edits.changes.size() == 1 && back.below[0].changes.size() == 2 &&
         back.below[1].changes.size() == 1);
  assert(back.editsOf(1).at(-7, 9)->placed && !back.editsOf(1).at(5, -2)->placed);
  assert(back.editsOf(2).at(100, -200)->piece == Piece::Wall && back.editsOf(0).at(3, 4));
  assert(!back.editsOf(0).at(-7, 9) && !back.editsOf(2).at(3, 4));
  std::string old = text.substr(0, text.find("layer "));
  Campaign before;
  assert(Campaign::load(old, before) && before.layer == 0 && before.below[0].changes.empty());
  Campaign wild;
  assert(Campaign::load(text + "layer 9\n", wild) && wild.layer == 2);
  std::cout << "[TEST] Campaign keeps one edit overlay per layer and the hero's layer: PASS" << std::endl;
}

void testCaves() {
  uint32_t salt = noise_salt;
  int kinds[5]{}, mouths = 0, ramps = 0, pools = 0, levels[4]{};
  long open = 0;
  for (uint32_t seed : {0u, 17u, 8675309u}) {
    noise_reseed(seed);
    world::enter(world::Layer::Caves);
    for (int cy = -6; cy < 6; cy++)
      for (int cx = -6; cx < 6; cx++) {
        world::CaveCell c, again;
        bool has = world::caveCell(cx, cy, c);
        world::caveCell(cx + 1, cy, again);
        world::caveCell(cx, cy, again);
        assert(again.kind == c.kind && again.size == c.size && again.mouths == c.mouths &&
               again.hx == c.hx && again.hy == c.hy);
        kinds[int(c.kind)]++;
        int x0 = cx * world::CAVE_CELL, y0 = cy * world::CAVE_CELL;
        if (!has) {
          for (int j = 0; j < world::CAVE_CELL; j += 5)
            for (int i = 0; i < world::CAVE_CELL; i += 5)
              assert(world::isRock(x0 + i, y0 + j));
          continue;
        }
        assert(c.size >= 200 && c.mouths >= 1 && !world::isRock(c.hx, c.hy));
        for (int k = 0; k < world::CAVE_CELL; k++)
          assert(world::isRock(x0 + k, y0) && world::isRock(x0, y0 + k) &&
                 world::isRock(x0 + k, y0 + world::CAVE_CELL - 1) &&
                 world::isRock(x0 + world::CAVE_CELL - 1, y0 + k));
        std::vector<char> seen(world::CAVE_CELL * world::CAVE_CELL, 0);
        std::vector<int> queue{(c.my[0] - y0) * world::CAVE_CELL + c.mx[0] - x0};
        seen[queue[0]] = 1;
        for (size_t q = 0; q < queue.size(); q++) {
          int i = queue[q] % world::CAVE_CELL, j = queue[q] / world::CAVE_CELL;
          for (int d = 0; d < 4; d++) {
            int ni = i + (d == 0) - (d == 1), nj = j + (d == 2) - (d == 3);
            int n = nj * world::CAVE_CELL + ni;
            if (!seen[n] && !world::isRock(x0 + ni, y0 + nj))
              seen[n] = 1, queue.push_back(n);
          }
        }
        assert(int(queue.size()) == c.size);
        int count = 0;
        for (int j = 0; j < world::CAVE_CELL; j++)
          for (int i = 0; i < world::CAVE_CELL; i++)
            if (!world::isRock(x0 + i, y0 + j)) {
              count++;
              levels[world::heightAt(x0 + i, y0 + j)]++;
              ramps += world::isRamp(x0 + i, y0 + j);
              pools += world::isWater(x0 + i, y0 + j);
            }
        assert(count == c.size);
        open += count;
        for (int m = 0; m < c.mouths; m++) {
          int x = c.mx[m], y = c.my[m];
          assert(!world::isRock(x, y) && !world::isWater(x, y) && seen[(y - y0) * world::CAVE_CELL + x - x0]);
          world::enter(world::Layer::Surface);
          Structure st;
          assert(!world::isWater(x, y) && !world::structureAt(x, y, st, 2));
          world::enter(world::Layer::Caves);
          mouths++;
        }
        if (c.kind == world::CaveKind::Through) {
          double dx = c.mx[1] - c.mx[0], dy = c.my[1] - c.my[0];
          assert(c.mouths == 2 && dx * dx + dy * dy >= 144);
        }
        if (c.kind == world::CaveKind::Descent)
          assert(!world::isRock(c.px, c.py));
      }
    world::enter(world::Layer::Surface);
  }
  noise_reseed(salt);
  for (int k = 1; k < 5; k++)
    assert(kinds[k] > 0);
  assert(levels[1] > 0 && levels[2] > 0 && ramps > 0 && pools > 0);
  std::cout << "[TEST] Caves: solid " << kinds[0] << ", pocket " << kinds[1] << ", through " << kinds[2]
            << ", warren " << kinds[3] << ", descent " << kinds[4] << " over 432 cells; " << mouths
            << " mouths all dry above and joined to their cave; " << open << " open tiles (levels "
            << levels[0] << "/" << levels[1] << "/" << levels[2] << ", " << ramps << " ramp tiles, "
            << pools << " pool tiles): PASS" << std::endl;
}

void testChartCorner() {
  Chart chart;
  chart.unveil(-24, -24, 0);
  assert(chart.at(-1, -1) != 0);
  std::string path = (std::filesystem::temp_directory_path() / "mellangan-chart-corner.bin").string();
  chart.save(path);
  Chart back;
  back.load(path);
  std::filesystem::remove(path);
  assert((back.at(-1, -1) & Chart::BIOME) == (chart.at(-1, -1) & Chart::BIOME) && back.at(-1, -1));
  std::cout << "[TEST] The chart finds the page just north-west of the origin on its first lookup: PASS" << std::endl;
}
