#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "entity/enemy/Enemy.hpp"
#include "level/Structure.hpp"

struct Codex {
  static constexpr int KINDS = int(EnemyKind::Count),
                       SITES = int(StructureKind::Count);

  uint32_t biomes = 0, relics = 0;
  int slain[KINDS]{}, entered[SITES]{};
  int runs = 0, dawns = 0, bestStreak = 0, start = 0, shrines = 0;
  float bestTime = 0;

  int kills() const;
  int count(uint32_t mask) const;
  bool parse(std::string_view text);
  void load(const std::string &path);
  void save(const std::string &path) const;
};
