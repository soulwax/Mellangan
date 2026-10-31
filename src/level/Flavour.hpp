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
  float rate, vx, vy, sway, spin, scale, life;
};

struct Omen {
  const char *name, *note;
  float step, foes, xp, elite, kindle;
  Color tint;
  int handicap = 1;
  float hp = 1, rate = 1, horde = 1, waves = 1, bands = 1, mimics = 1,
        skilled = 0.35f;
};

constexpr int OMEN_COUNT = 10;
extern const Flavour FLAVOURS[BIOME_COUNT];
extern const Omen OMENS[OMEN_COUNT];
extern const Omen CALM;
extern const Air AIRS[BIOME_COUNT];
