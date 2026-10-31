#include "BiomeTransitionTest.hpp"

#include <cassert>
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

bool exotic(BiomeId b) {
  return b == BIOME_BARROW || b == BIOME_ASHLANDS || b == BIOME_CRYSTAL ||
         b == BIOME_FUNGAL || b >= BIOME_SALT;
}

void distanceDistribution() {
  int count[3]{}, unusual[3]{};
  for (uint32_t seed : {0u, 17u, 19u, 837u, 8675309u}) {
    noise_reseed(seed);
    for (int y = -1000; y <= 1000; y += 25)
      for (int x = -1000; x <= 1000; x += 25) {
        double d = std::hypot(double(x), double(y));
        BiomeId b = world::biomeAt(x, y);
        if (d <= 90)
          assert(!exotic(b));
        int band = d < 180 ? 0 : d < 450 ? 1 : d <= 1000 ? 2 : -1;
        if (band >= 0)
          count[band]++, unusual[band] += exotic(b);
        int sx, sy;
        world::siteOf(x, y, sx, sy);
        if (std::hypot(double(sx), double(sy)) >= 660)
          assert(world::biomeForLocation(x, y) == world::biomeOf(world::climateAt(sx, sy)));
      }
  }
  assert(unusual[2] > 0);
  assert(double(unusual[0]) / count[0] < double(unusual[2]) / count[2] * 0.5);
  std::cout << "[TEST] Exotic biome samples near/middle/far: "
            << unusual[0] << "/" << count[0] << ", " << unusual[1] << "/"
            << count[1] << ", " << unusual[2] << "/" << count[2] << std::endl;
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
  float maxStep = 0;
  for (int y = -512; y <= 512 && !boundary; y++)
    for (int x = -512; x < 512; x++)
      if (world::biomeAt(x, y) != world::biomeAt(x + 1, y)) {
        boundary = true;
        for (int d = -8; d < 8; d++) {
          GroundBlend a = checkedBlend(x + d, y), b = checkedBlend(x + d + 1, y);
          maxStep = std::max(maxStep, blendDistance(a, b));
        }
        break;
      }
  assert(boundary);
  for (int y = -512; y <= 512; y += 8)
    for (int x = -512; x <= 512; x += 8) {
      GroundBlend a = checkedBlend(x - 1, y), b = checkedBlend(x, y),
                  c = checkedBlend(x + 1, y), d = checkedBlend(x, y - 1),
                  e = checkedBlend(x, y + 1);
      maxStep = std::max({maxStep, blendDistance(a, b), blendDistance(b, c),
                          blendDistance(b, d), blendDistance(b, e)});
      if (a.count == 1 && a.weight[0] == 1)
        interior = true;
    }
  assert(interior);
  std::cout << "[TEST] Maximum adjacent blend difference: " << maxStep << std::endl;
  assert(maxStep <= 0.9f);
  int oldMixed = 0, newMixed = 0;
  for (int y = -384; y <= 384; y += 8)
    for (int x = -384; x <= 384; x += 8) {
      BiomeId first = world::biomeAt(x - 8, y - 8);
      bool mixed = false;
      for (int iy = -2; iy <= 2; iy++)
        for (int ix = -2; ix <= 2; ix++)
          mixed |= world::biomeAt(x + ix * 4, y + iy * 4) != first;
      oldMixed += mixed;
      newMixed += world::groundBlend(x, y).count > 1;
    }
  std::cout << "[TEST] Mixed biome samples at old/new radius: " << oldMixed
            << "/" << newMixed << std::endl;
  assert(oldMixed > 0 && newMixed * 100 <= oldMixed * 70);
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
  assert(newMagic == 0x3543414Du);
  Chart roundTrip;
  roundTrip.load(saved.string());
  assert(roundTrip.at(0, 0) == current + 1);
  assert(roundTrip.at(1, 0) == 0);
  header[0] = 0x3443414Du;
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char *>(header), sizeof header);
    out.write(reinterpret_cast<const char *>(&px), sizeof px);
    out.write(reinterpret_cast<const char *>(&py), sizeof py);
    out.write(reinterpret_cast<const char *>(cells), sizeof cells);
    assert(out.good());
  }
  Chart scaled;
  scaled.load(path.string());
  assert(scaled.at(0, 0) == current + 1 && scaled.at(1, 0) == 0);
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
  distanceDistribution();
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
      Climate old = legacyClimateAt(x / world::CLIMATE_SCALE, y / world::CLIMATE_SCALE);
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
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(p.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(p.y - game.screen.vh / 2.0));
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

void viewDeathEffects(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &player = *level.player;
  player.god = true;
  level.night.dawn = true;
  const EnemyKind kinds[] = {EnemyKind::Rat, EnemyKind::Ghost, EnemyKind::Demon,
                             EnemyKind::Slime, EnemyKind::Bat, EnemyKind::Spider};
  bool running = true;
  uint64_t ends = SDL_GetTicks() + 20000;
  for (int frame = 0; running && SDL_GetTicks() < ends; frame++) {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
    if (frame % 120 == 0)
      for (int i = 0; i < 6; i++) {
        auto enemy = Enemy::create(kinds[i], 1, 1);
        Enemy *target = enemy.get();
        target->x = player.x - 150 + i * 60;
        target->y = player.y;
        target->z = i == 4 ? 7 : 0;
        level.add(std::move(enemy));
        target->hurt(target->hp + 1, 0, 0, 0);
      }
    level.tick();
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(player.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(player.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.35f, 0.35f, 0.35f, 1});
    level.renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level.render(game.screen);
    enemyWarning(game.screen, player.x, player.y + 58, 24, 0.9f, 0);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    const char *labels[] = {"RAT", "GHOST", "DEMON", "SLIME", "BAT", "SPIDER"};
    for (int i = 0; i < 6; i++)
      game.screen.textCenter(labels[i], 90.0f + i * 60, 95, 0.8f, col::white);
    game.screen.textCenter("ENEMY WARNING", 240, 205, 0.8f, col::red);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}

void viewGui(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  LevelUpMenu upgrades(game);
  ShrineMenu shrine(game);
  upgrades.t = shrine.t = 1;
  uint64_t start = SDL_GetTicks();
  bool running = true;
  while (running && SDL_GetTicks() - start < 20000) {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
    Player &p = *game.level->player;
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(p.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(p.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.35f, 0.35f, 0.35f, 1});
    game.level->renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    game.level->render(game.screen);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    if ((SDL_GetTicks() - start) / 5000 % 2 == 0)
      upgrades.render(game.screen);
    else
      shrine.render(game.screen);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}

void viewBiomeEncounter(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &upgrade : p.upgrades)
    upgrade->level = 0;
  level.night.dawn = true;
  level.time = 120;
  int tx = 0, ty = 0;
  bool found = false;
  for (int y = -300; y <= 300 && !found; y += 20)
    for (int x = -300; x <= 300 && !found; x += 20)
      if (world::biomeAt(x, y) == BIOME_FOREST)
        tx = x, ty = y, found = true;
  p.x = tx * 16, p.y = ty * 16;
  Structure site{StructureKind::Ruin, tx - 1, ty - 1, 2, 2, 0, 0, 0};
  level.add<ExpansionChallenge>(site, BIOME_FOREST);
  bool running = true;
  uint64_t end = SDL_GetTicks() + 20000;
  while (running && SDL_GetTicks() < end) {
    SDL_Event e;
    while (SDL_PollEvent(&e))
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
    level.tick();
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(p.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(p.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.35f, 0.35f, 0.35f, 1});
    level.renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level.render(game.screen);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    game.screen.textCenter("FOREST CHALLENGE: DEFEAT THE GUARDS", game.screen.w / 2,
                           16 * game.screen.u, game.screen.u, col::gold);
    game.screen.textCenter(std::string("REWARD: ") + biomeRewardText(BIOME_FOREST),
                           game.screen.w / 2, 34 * game.screen.u, game.screen.u, col::white);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}

void viewCombatReadability(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.screen.load(base))
    return;
  SDL_SetWindowAlwaysOnTop(window, true);
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &upgrade : p.upgrades)
    upgrade->level = 0;
  level.night.dawn = true;
  for (int i = 0; i < int(StatusKind::Count); i++)
    p.afflict(StatusKind(i), 60, 0.1f);
  Melee windup(2, 0.4f, 1, 1);
  auto enemy = Enemy::create(EnemyKind::Rat, 10, 1);
  Enemy *target = enemy.get();
  target->x = p.x + 60;
  target->y = p.y;
  target->plan = nullptr;
  target->goals.clear();
  target->goals.push(&windup);
  target->goalTime = 1;
  target->xMark = -1;
  target->yMark = 0;
  for (int i = 0; i < int(StatusKind::Count); i++)
    target->afflict(StatusKind(i), 60, 0.1f);
  level.add(std::move(enemy));
  bool running = true;
  uint64_t end = SDL_GetTicks() + 20000;
  int frame = 0;
  while (running && SDL_GetTicks() < end) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
      if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_V)
        game.reducedParticles = !game.reducedParticles;
    }
    if (frame % 180 == 0)
      level.add<Orb>(p.x - 50, p.y - 12, 0, 0, 0);
    if (frame % 20 == 0)
      Spark::burst(level, p.x, p.y - 15, col::gold, 14, 100, true);
    level.tick();
    game.screen.fit(window);
    game.screen.xOffset = int(std::floor(p.x - game.screen.vw / 2.0));
    game.screen.yOffset = int(std::floor(p.y - game.screen.vh / 2.0));
    game.screen.beginLight({0.3f, 0.3f, 0.3f, 1});
    level.renderLight(game.screen);
    game.screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level.render(game.screen);
    enemyWarning(game.screen, p.x + 110, p.y, 18, 0.9f, 0);
    game.screen.beginGui(window, {0, 0, 0, 0, 0});
    game.screen.textCenter(game.reducedParticles ? "V: REDUCED PARTICLES" : "V: FULL PARTICLES",
                           game.screen.w / 2, 12 * game.screen.u, game.screen.u, col::white);
    vk2dRendererPresent();
    SDL_Delay(16);
    frame++;
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}

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
  FILE *f = std::fopen(path, "wb");
  if (!f)
    return 1;
  std::fprintf(f, "P6 %d %d 255\n", SIDE, SIDE);
  std::fwrite(rgb.data(), 1, rgb.size(), f);
  std::fclose(f);
  return 0;
}

void captureScreenshotSuite(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  if (!game.init(window, base))
    return;

  VK2DTexture cap = vk2dTextureCreate(1440, 810);
  if (!cap) {
    game.shutdown();
    return;
  }

  auto snap = [&](const char *path) {
    game.screen.targetGui(cap);
    game.render();
    game.screen.targetGui(nullptr);
    vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
    vk2dRendererPresent();
    vk2dRendererWait();
    game.screen.saveTarget(cap, path);
  };

  game.worldMode = WorldMode::Random;
  game.seedCode = "KXXCCTRM";
  game.reseed();
  for (int i = 0; i < 40; i++) {
    game.tick();
    game.render();
    vk2dRendererPresent();
  }
  snap("static/screenshots/01-title.png");

  game.codex.start = 0;
  game.setMenu(std::make_unique<ClassMenu>(game));
  for (int i = 0; i < 40; i++) {
    game.tick();
    game.render();
    vk2dRendererPresent();
  }
  snap("static/screenshots/02-classes.png");

  game.menu.reset();
  game.dev = true;
  game.biomePick = BIOME_SAVANNA;
  game.visitBiome();
  Player &p = *game.level->player;
  p.hero = HEROES[0];
  HEROES[0]->equip(p);
  p.god = true;
  game.level->time = 100.0f;
  game.level->kills = 164;
  game.level->streak = 150;
  game.level->streakTime = 2.0f;
  p.lvl = 9;
  p.exp = 280;
  p.expNext = 360;
  p.hp = 100;
  p.maxHp = 100;
  game.banner = "THE SAVANNA";
  game.bannerTime = 2.0f;

  for (auto &up : p.upgrades) {
    std::string_view n = up->name;
    if (n == "Staff" || n == "Frost Nova" || n == "Meteor" ||
        n == "Fire Bomb" || n == "Transmute" || n == "Boots" ||
        n == "Amulet" || n == "Tome")
      up->level = std::max(1, up->level);
  }

  for (int i = 0; i < 8; i++) {
    float a = float(i) * 6.28318f / 8.0f;
    float dist = 70.0f + (i % 3) * 20.0f;
    EnemyKind k = (i % 4 == 0) ? EnemyKind::Wolf : (i % 4 == 1) ? EnemyKind::Ghost : (i % 4 == 2) ? EnemyKind::Rat : EnemyKind::Slime;
    auto e = Enemy::create(k, 10, 1);
    e->x = p.x + std::cos(a) * dist;
    e->y = p.y + std::sin(a) * dist;
    game.level->add(std::move(e));
  }

  for (int i = 0; i < 3; i++) {
    float a = float(i) * 6.28318f / 3.0f;
    game.level->add<Orb>(float(p.x + std::cos(a) * 35.0), float(p.y + std::sin(a) * 35.0), 0, 0, 0);
  }
  for (int i = 0; i < 6; i++) {
    float gx = float(p.x + (i - 3) * 18.0 + 10.0);
    float gy = float(p.y + ((i * 7) % 5 - 2) * 15.0);
    game.level->add<Gem>(gx, gy, 5);
  }
  Spark::burst(*game.level, float(p.x + 25), float(p.y - 10), col::gold, 16, 80, true);
  Spark::burst(*game.level, float(p.x - 30), float(p.y + 15), col::xp, 16, 80, true);

  for (int i = 0; i < 8; i++) {
    game.level->tick();
    game.render();
    vk2dRendererPresent();
  }

  game.screen.targetGui(cap);
  game.render();
  enemyWarning(game.screen, float(p.x + 55), float(p.y - 20), 22.0f, 0.9f, 0.0f);
  game.screen.targetGui(nullptr);
  vk2dRendererSetTarget(VK2D_TARGET_SCREEN);
  vk2dRendererPresent();
  vk2dRendererWait();
  game.screen.saveTarget(cap, "static/screenshots/03-combat.png");

  for (auto &up : p.upgrades)
    up->level = 0;
  for (auto &up : p.upgrades) {
    if (std::string_view(up->name) == "Staff")
      up->level = 1;
    if (std::string_view(up->name) == "Frost Nova")
      up->level = 1;
  }
  p.lvl = 2;
  p.exp = 0;
  p.expNext = 100;
  p.pendingLevels = 1;
  game.setMenu(std::make_unique<LevelUpMenu>(game));
  for (int i = 0; i < 30; i++) {
    game.tick();
    game.render();
    vk2dRendererPresent();
  }
  snap("static/screenshots/04-upgrades.png");

  vk2dTextureFree(cap);
  game.shutdown();
}
