#pragma once

#include <cstdint>

#include "entity/enemy/Enemy.hpp"

enum class Faction : uint8_t {
  Wildkin,
  BarrowHost,
  Mycel,
  Cinderbound,
  Rimefolk,
  Glasskin,
  Bogspawn,
  Starborn,
  Freeblades,
  Synod,
  Hearth,
  Count
};

constexpr int FAMILIES = int(Faction::Synod);

constexpr Faction UNCLEAN[] = {Faction::Mycel, Faction::Starborn, Faction::Glasskin, Faction::Bogspawn,
                               Faction::BarrowHost};
bool unclean(Faction f);
Faction factionOf(EnemyKind kind);
Faction factionOf(const Enemy &e);
const char *factionName(Faction f);

struct Grudges {
  static constexpr int MAX = 3;
  Faction pair[MAX][2];
  int count = 0;

  static Grudges roll(uint32_t seed);
  bool between(Faction a, Faction b) const;
};
