#include "DevStart.hpp"

#include <spdlog/spdlog.h>

#include <cstdio>
#include <cstdlib>
#include <string_view>

#include "entity/Ally.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "item/loot/Loot.hpp"
#include "level/World.hpp"

void DevStart::parse(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    std::string_view a = argv[i];
    bool more = i + 1 < argc;
    if (a == "--arm" && more)
      arm = argv[++i];
    else if (a == "--hero" && more)
      hero = std::atoi(argv[++i]);
    else if (a == "--biome" && more)
      biome = std::atoi(argv[++i]);
    else if (a == "--give" && more)
      give = std::atoi(argv[++i]);
    else if (a == "--allies" && more)
      allies = std::atoi(argv[++i]);
    else if (a == "--landmark" && more)
      landmark = std::atoi(argv[++i]);
    else if (a == "--at" && more)
      at = std::sscanf(argv[++i], "%d,%d", &x, &y) == 2;
    lake |= a == "--lake";
    cliff |= a == "--cliff";
    ramp |= a == "--ramp";
    peak |= a == "--peak";
  }
}

template <class F> void DevStart::seek(F fits) {
  for (int r = 0; r < 2000; r += 4)
    for (int i = 0, n = std::max(1, r); i < n; i++) {
      double a = TAU * i / n;
      int tx = int(std::cos(a) * r), ty = int(std::sin(a) * r);
      if (fits(tx, ty)) {
        x = tx, y = ty, at = true;
        return;
      }
    }
}

void DevStart::seekLandmark() {
  for (int r = 0; r < 40 && !at; r++)
    for (int ry = -r; ry <= r && !at; ry++)
      for (int rx = -r; rx <= r && !at; rx++) {
        Structure site;
        if ((std::abs(rx) == r || std::abs(ry) == r) &&
            world::structureIn(rx, ry, site) &&
            int(site.kind) == int(StructureKind::AncientTree) + landmark)
          x = site.x + 3, y = site.y + site.h + 2, at = true;
      }
}

void DevStart::apply(Player &p) {
  if (!arm.empty())
    armPlayer(p);
  for (int i = 0; i < give && int(p.bag.size()) < Player::BAG; i++)
    p.bag.push_back(rollItem(30, 300, true, rng));
  if (biome >= 0 && biome < BIOME_COUNT)
    visitBiome(p);
  if (lake)
    seek([](int x, int y) {
      return world::isWater(x, y) && world::isWater(x, y - 2) &&
             !world::isWater(x, y + 3);
    });
  if (ramp)
    seek([&](int x, int y) {
      return world::isRamp(x, y - 1) && world::isRamp(x - 1, y - 1) &&
             world::isRamp(x + 1, y - 1) && !world::isWater(x, y + 1) &&
             (biome < 0 || world::biomeAt(x, y) == biome);
    });
  if (cliff)
    seek([&](int x, int y) {
      if (biome >= 0 && world::biomeAt(x, y) != biome)
        return false;
      for (int dx = -2; dx <= 2; dx++)
        if (world::heightAt(x + dx, y - 1) <= world::heightAt(x + dx, y))
          return false;
      return world::heightAt(x, y + 2) == world::heightAt(x, y) &&
             !world::isWater(x, y + 2);
    });
  if (peak)
    seek([](int x, int y) {
      return world::heightAt(x, y) == 3 && world::heightAt(x, y + 6) < 3;
    });
  if (landmark >= 0)
    seekLandmark();
  if (at) {
    p.x = x * 16 + 8, p.y = y * 16 + 8;
    spdlog::info("dev: starting at tile ({}, {})", x, y);
  }
  for (int i = 0; i < allies && p.level; i++) {
    double a = TAU * i / allies;
    summon(*p.level, AllyKind(i % int(AllyKind::Decoy)), p.x + std::cos(a) * 24,
           p.y + std::sin(a) * 16, 12, 120);
  }
}

bool DevStart::visitBiome(Player &p) const {
  int tx, ty;
  if (!world::findBiome(biome, tx, ty)) {
    spdlog::warn("dev: biome {} not found", biome);
    return false;
  }
  p.x = tx * 16 + 8, p.y = ty * 16 + 8;
  spdlog::info("dev: visiting {} at tile ({}, {})", BIOMES[biome]->name, tx, ty);
  return true;
}

void DevStart::armPlayer(Player &p) const {
  std::string_view list = arm;
  if (list.front() == '=') {
    list.remove_prefix(1);
    for (auto &u : p.upgrades)
      u->level = 0;
  }
  while (!list.empty()) {
    size_t comma = list.find(',');
    std::string_view name = list.substr(0, comma);
    list.remove_prefix(comma == list.npos ? list.size() : comma + 1);
    bool evolve = !name.empty() && name.back() == '!';
    if (evolve)
      name.remove_suffix(1);
    Upgrade *found = p.upgrade(name);
    if (!found) {
      spdlog::warn("dev: no upgrade named {}", name);
      continue;
    }
    Upgrade &u = *found;
    while (!u.maxed())
      u.levelUp(p);
    if (evolve && u.evolvedName)
      u.evolve();
    spdlog::info("dev: armed {} at level {}", u.name, u.level);
  }
}
