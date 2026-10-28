#pragma once

#include "level/Structure.hpp"

class Level;
class Screen;

class Night {
public:
  static constexpr float LENGTH = 900, PULL = 60, KINDLE = 6;
  static constexpr int MAX_LIT = 64;

  int lit = 0;
  float kindling = 0;
  bool dawn = false, hasBeacon = false;
  bool offering = false;
  Structure beacon{};

  explicit Night(Level &level) : level(level) {}
  void tick();
  void renderLight(Screen &s) const;
  float dawnAt() const { return LENGTH - lit * PULL; }
  float left() const;
  float daylight() const;
  bool isLit(const Structure &s) const;

private:
  Level &level;
  Structure shrines[MAX_LIT]{}, current{};
  float scan = 0;

  void kindle();
  static bool cold(const Structure *s, const void *night);
};
