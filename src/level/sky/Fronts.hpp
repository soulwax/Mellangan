#pragma once

#include "gfx/Screen.hpp"

enum class Front { Clear, Clouds, Rain, Storm, Snow, Fog, Dust, Ash, Count };
enum class Fall { None, Rain, Flake, Streak };

struct FrontDef {
  int drops;
  Fall fall;
  float speed, sway, wind;
  Color tint, shade;
  float clouds, haze;
  Color mist;
  float patchy;
  const char *name;
};

constexpr int FRONTS = int(Front::Count);

const FrontDef &frontDef(Front f);
int frontChance(int biome, Front f);
Front pickFront(int biome, Front current);
Front groundLayer(Front f);
