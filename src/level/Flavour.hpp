#pragma once

#include "gfx/Screen.hpp"
#include "level/biome/Biome.hpp"

struct Flavour {
  float step, foes, xp, mend;
  bool burn;
  const char *note;
};

struct Air {
  Color grade;
  int icon;
  Color tint;
  float rate, vx, vy, sway, spin, scale, life, strike;
};

struct Omen {
  const char *name, *note;
  float step, foes, xp, elite, kindle;
  Color tint;
};

constexpr int OMEN_COUNT = 3;
extern const Flavour FLAVOURS[BIOME_COUNT];
extern const Omen OMENS[OMEN_COUNT];
extern const Air AIRS[BIOME_COUNT];
