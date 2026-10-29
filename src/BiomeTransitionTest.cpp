#include "BiomeTransitionTest.hpp"

#include <cassert>
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

namespace {
using world::GroundBlend;

Climate legacyClimateAt(float x, float y) {
  float wx = x + noise_fbm(x * 0.045f + 11.3f, y * 0.045f - 4.1f, 2, 701) * 5 +
             noise_perlin(x * 0.23f + 3.1f, y * 0.23f - 8.7f, 703) * 0.9f;
  float wy = y + noise_fbm(x * 0.045f - 7.7f, y * 0.045f + 9.2f, 2, 702) * 5 +
             noise_perlin(x * 0.23f - 5.3f, y * 0.23f + 2.9f, 704) * 0.9f;
  return {noise_fbm(wx * 0.011f, wy * 0.011f, 3, 101),
          noise_fbm(wx * 0.013f + 40.5f, wy * 0.013f, 3, 202),
          noise_fbm(wx * 0.017f, wy * 0.017f - 60.5f, 2, 303)};
}

BiomeId legacyBiomeOf(Climate c) {
  if (c.eerie > -0.18f && c.eerie < -0.08f) {
    int band = c.temp < -0.2f ? 0 : c.temp < -0.07f ? 1 :
               c.temp < 0.07f ? 2 : c.temp < 0.2f ? 3 : 4;
    return BiomeId(BIOME_SALT + band * 2 + (c.moist > 0 ? 1 : 0));
  }
  if (c.eerie > 0.14f && c.temp > 0.2f)
    return BIOME_ASHLANDS;
  if (c.eerie > 0.21f && c.temp < -0.14f)
    return BIOME_CRYSTAL;
  if (c.eerie > 0.14f && c.moist > 0.18f)
    return BIOME_FUNGAL;
  if (c.eerie > 0.26f)
    return BIOME_BARROW;
  if (c.temp < -0.12f && c.temp > -0.38f && c.moist > 0.08f && c.moist < 0.26f)
    return BIOME_TAIGA;
  if (c.temp < -0.24f)
    return BIOME_SNOW;
  if (c.temp > 0.15f && c.moist > 0.12f)
    return BIOME_JUNGLE;
  if (c.temp > 0.0f && c.temp < 0.3f && c.moist < -0.1f && c.eerie < -0.14f)
    return BIOME_BADLANDS;
  if (c.temp > 0.1f && c.moist < -0.16f)
    return BIOME_DESERT;
  if (c.moist < -0.18f)
    return BIOME_HEATH;
  if (c.temp > 0.17f && c.moist < -0.06f)
    return BIOME_SAVANNA;
  if (c.temp > 0.18f && c.moist <= 0.03f)
    return BIOME_BLOSSOM;
  if (c.moist > 0.24f && c.temp < 0.08f)
    return BIOME_MIRE;
  if (c.moist > 0.03f)
    return c.temp > 0.0f ? BIOME_AUTUMN : BIOME_FOREST;
  return BIOME_MEADOW;
}

struct LegacyMemo {
  int x, y;
  uint32_t epoch;
  BiomeId biome;
};

BiomeId legacyRawBiomeAt(int x, int y) {
  static LegacyMemo memo[128 * 64];
  LegacyMemo &m = memo[(x & 127) | (y & 63) << 7];
  if (m.epoch == noise_epoch && m.x == x && m.y == y)
    return m.biome;
  m = {x, y, noise_epoch, legacyBiomeOf(legacyClimateAt(x, y))};
  return m.biome;
}

BiomeId legacyBiomeAt(int x, int y) {
  static LegacyMemo memo[128 * 64];
  LegacyMemo &m = memo[(x & 127) | (y & 63) << 7];
  if (m.epoch == noise_epoch && m.x == x && m.y == y)
    return m.biome;
  BiomeId own = legacyRawBiomeAt(x, y), best = own;
  uint8_t votes[BIOME_COUNT]{};
  for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++) {
      BiomeId b = legacyRawBiomeAt(x + dx, y + dy);
      if (++votes[b] > votes[best] ||
          (b == own && votes[b] == votes[best]))
        best = b;
    }
  m = {x, y, noise_epoch, best};
  return best;
}

struct SaltRestore {
  uint32_t salt = noise_salt;
  ~SaltRestore() { noise_reseed(salt); }
};

GroundBlend checkedBlend(int x, int y) {
  GroundBlend b = world::groundBlend(x, y);
  assert(b.count >= 1 && b.count <= 3);
  float sum = 0;
  bool seen[BIOME_COUNT]{};
  for (int i = 0; i < b.count; i++) {
    assert(b.biome[i] >= 0 && b.biome[i] < BIOME_COUNT);
    assert(!seen[b.biome[i]]);
    seen[b.biome[i]] = true;
    assert(std::isfinite(b.weight[i]) && b.weight[i] >= 0 && b.weight[i] <= 1);
    sum += b.weight[i];
  }
  assert(std::abs(sum - 1) < 0.0001f);
  return b;
}

float weightAt(const GroundBlend &b, int id) {
  for (int i = 0; i < b.count; i++)
    if (b.biome[i] == id)
      return b.weight[i];
  return 0;
}

float blendDistance(const GroundBlend &a, const GroundBlend &b) {
  float d = 0;
  for (int id = 0; id < BIOME_COUNT; id++)
    d += std::abs(weightAt(a, id) - weightAt(b, id));
  return d;
}

void exportPreview(const char *path) {
  if (!path || !path[0])
    return;
  std::filesystem::path file(path);
  if (file.has_parent_path())
    std::filesystem::create_directories(file.parent_path());
  std::ofstream out(file);
  assert(out.good());
  out << "record,x,y,old,new,count,id0,w0,id1,w1,id2,w2\n";
  for (int id = 0; id < BIOME_COUNT; id++)
    for (int kind = 0; kind < 3; kind++) {
      Sprite s = BIOMES[id]->transitionGround(kind);
      out << "sprite," << id << ',' << kind << ',' << s.x << ',' << s.y << ','
          << s.w << ',' << s.h << ",,,,,\n";
    }
  for (int y = -192; y < 192; y++)
    for (int x = -256; x < 256; x++) {
      GroundBlend b = checkedBlend(x, y);
      out << "tile," << x << ',' << y << ',' << legacyBiomeAt(x, y) << ','
          << world::biomeAt(x, y) << ',' << b.count;
      for (int i = 0; i < 3; i++)
        out << ',' << (i < b.count ? int(b.biome[i]) : -1) << ','
            << (i < b.count ? b.weight[i] : 0);
      out << '\n';
    }
  assert(out.good());
}

void reachability() {
  std::array<bool, BIOME_COUNT> seen{};
  for (int y = -4000; y <= 4000; y += 41)
    for (int x = -4000; x <= 4000; x += 41)
      seen[world::biomeAt(x, y)] = true;
  for (bool found : seen)
    assert(found);
  std::cout << "[TEST] All 26 biomes reachable in bounded seeded survey: PASS"
            << std::endl;
}

void crossings() {
  constexpr int SEEDS[] = {0, 17, 8675309};
  constexpr int BOUND = 2048, LINES = 32;
  uint64_t oldCount = 0, newCount = 0;
  for (uint32_t seed : SEEDS) {
    noise_reseed(seed);
    uint64_t before = oldCount, after = newCount;
    for (int line = 0; line < LINES; line++) {
      int offset = -BOUND + line * (2 * BOUND / LINES);
      for (int n = -BOUND; n < BOUND; n++) {
        BiomeId oldX = legacyBiomeAt(n, offset), nowX = world::biomeAt(n, offset);
        oldCount += oldX != legacyBiomeAt(n + 1, offset);
        newCount += nowX != world::biomeAt(n + 1, offset);
        BiomeId oldY = legacyBiomeAt(offset, n), nowY = world::biomeAt(offset, n);
        oldCount += oldY != legacyBiomeAt(offset, n + 1);
        newCount += nowY != world::biomeAt(offset, n + 1);
      }
    }
    std::cout << "[TEST] Seed " << seed << " adjacent crossings: legacy "
              << oldCount - before << ", scaled " << newCount - after
              << std::endl;
  }
  std::cout << "[TEST] Aggregate adjacent crossings: legacy " << oldCount
            << ", scaled " << newCount << std::endl;
  assert(oldCount > 0 && newCount * 100 <= oldCount * 65);
}

void blendChecks() {
  bool interior = false, boundary = false;
  for (int y = -512; y <= 512 && !boundary; y++)
    for (int x = -512; x < 512; x++)
      if (world::biomeAt(x, y) != world::biomeAt(x + 1, y)) {
        boundary = true;
        for (int d = -8; d < 8; d++) {
          GroundBlend a = checkedBlend(x + d, y), b = checkedBlend(x + d + 1, y);
          assert(blendDistance(a, b) <= 0.8f);
        }
        break;
      }
  assert(boundary);
  for (int y = -512; y <= 512; y += 8)
    for (int x = -512; x <= 512; x += 8) {
      GroundBlend a = checkedBlend(x - 1, y), b = checkedBlend(x, y),
                  c = checkedBlend(x + 1, y), d = checkedBlend(x, y - 1),
                  e = checkedBlend(x, y + 1);
      assert(blendDistance(a, b) <= 0.8f);
      assert(blendDistance(b, c) <= 0.8f);
      assert(blendDistance(b, d) <= 0.8f);
      assert(blendDistance(b, e) <= 0.8f);
      if (a.count == 1 && a.weight[0] == 1)
        interior = true;
    }
  assert(interior);
  for (int id = 0; id < BIOME_COUNT; id++)
    for (int kind = 0; kind < 3; kind++) {
      Sprite s = BIOMES[id]->transitionGround(kind);
      assert(std::isfinite(s.x) && std::isfinite(s.y) &&
             std::isfinite(s.w) && std::isfinite(s.h));
      assert(s.x >= 0 && s.y >= 0 && s.w > 0 && s.h > 0 &&
             s.x + s.w <= 432 && s.y + s.h <= 786);
    }
  std::cout << "[TEST] Ground blend normalization, interiors, boundaries, lattice seams and atlas rects: PASS"
            << std::endl;
}

void chartMigration() {
  std::filesystem::path path = std::filesystem::temp_directory_path() /
      ("mellangan-biome-chart-" + std::to_string(SDL_GetTicksNS()) + ".bin");
  std::filesystem::path saved = path.string() + ".saved";
  uint32_t header[3] = {0x3343414Du, 1, 0};
  int32_t px = 0, py = 0;
  uint8_t cells[Chart::PAGE * Chart::PAGE]{};
  int current = int(world::biomeAt(2, 2));
  cells[0] = uint8_t((current + 1) % BIOME_COUNT + 1);
  {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char *>(header), sizeof header);
    out.write(reinterpret_cast<const char *>(&px), sizeof px);
    out.write(reinterpret_cast<const char *>(&py), sizeof py);
    out.write(reinterpret_cast<const char *>(cells), sizeof cells);
    assert(out.good());
  }
  Chart chart;
  chart.load(path.string());
  assert(chart.at(0, 0) == current + 1);
  assert(chart.at(1, 0) == 0);
  chart.save(saved.string());
  uint32_t newMagic = 0;
  {
    std::ifstream in(saved, std::ios::binary);
    in.read(reinterpret_cast<char *>(&newMagic), sizeof newMagic);
    assert(in.good());
  }
  assert(newMagic == 0x3443414Du);
  Chart roundTrip;
  roundTrip.load(saved.string());
  assert(roundTrip.at(0, 0) == current + 1);
  assert(roundTrip.at(1, 0) == 0);
  std::filesystem::remove(path);
  std::filesystem::remove(saved);
  std::cout << "[TEST] Legacy biome chart migration and current-format round trip: PASS"
            << std::endl;
}
}

void testBiomeTransitions() {
  SaltRestore restore;
  noise_reseed(0);
  reachability();
  for (int salt : {0, 19, 8675309}) {
    noise_reseed(uint32_t(salt));
    BiomeId a = world::biomeAt(-731, -419), b = world::biomeAt(931, -177);
    assert(a == world::biomeAt(-731, -419));
    assert(b == world::biomeAt(931, -177));
    GroundBlend g = checkedBlend(-731, -419);
    world::biomeAt(-731 + 128, -419 + 64);
    world::groundBlend(-731 + 256, -419 + 256);
    assert(blendDistance(g, checkedBlend(-731, -419)) == 0);
  }
  noise_reseed(0);
  for (int x : {-101, -1, 0, 1, 101})
    for (int y : {-37, 0, 43}) {
      Climate scaled = world::climateAt(x, y);
      Climate old = legacyClimateAt(x / 2.5f, y / 2.5f);
      assert(std::abs(scaled.temp - old.temp) < 0.000001f);
      assert(std::abs(scaled.moist - old.moist) < 0.000001f);
      assert(std::abs(scaled.eerie - old.eerie) < 0.000001f);
    }
  std::array<BiomeId, 256> baseline{};
  for (int i = 0; i < 256; i++)
    baseline[i] = world::biomeAt(i * 19 - 1900, i * 11 - 1300);
  noise_reseed(19);
  bool reseeded = false;
  for (int i = 0; i < 256; i++)
    reseeded |= baseline[i] != world::biomeAt(i * 19 - 1900, i * 11 - 1300);
  assert(reseeded);
  crossings();
  noise_reseed(0);
  blendChecks();
  chartMigration();
  exportPreview(std::getenv("MELLANGAN_BIOME_PREVIEW"));
  std::cout << "[TEST] Biome transitions deterministic, reseedable, negative-coordinate and cache-eviction checks: PASS"
            << std::endl;
}

void viewBiomeTransition(SDL_Window *window, const std::string &base, int x,
                         int y, uint32_t seed) {
  SaltRestore restore;
  noise_reseed(seed);
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  std::cout << "[VIEW] Resources loaded" << std::endl;
  game.level = std::make_unique<Level>(game);
  Player &p = *game.level->player;
  p.x = x * 16 + 8;
  p.y = y * 16 + 8;
  p.god = true;
  bool running = true;
  uint64_t ends = SDL_GetTicks() + 20000;
  int frame = 0;
  for (; frame < 1800 && running && SDL_GetTicks() < ends; frame++) {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
    game.screen.xOffset = int(std::floor(p.x - Screen::W / 2.0));
    game.screen.yOffset = int(std::floor(p.y - Screen::H / 2.0));
    game.screen.beginLight({0.3f, 0.3f, 0.3f, 1});
    game.level->renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    game.level->render(game.screen);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    BiomeId id = world::biomeAt(x, y);
    game.screen.text("BIOME TRANSITION QA", 8, 8, 1, col::gold);
    game.screen.text(fmt::format("tile {},{}  seed {}  {} ({})", x, y, seed,
                                 BIOMES[id]->name, int(id)),
                     8, 20, 1, col::white);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  std::cout << "[VIEW] Rendered " << frame << " frames; releasing level" << std::endl;
  game.level.reset();
  std::cout << "[VIEW] Releasing screen" << std::endl;
  game.screen.unload();
  std::cout << "[VIEW] Screen released" << std::endl;
}
