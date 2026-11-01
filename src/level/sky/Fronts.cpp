#include "level/sky/Fronts.hpp"

#include "level/biome/Biome.hpp"

namespace {

using enum Fall;

const FrontDef DEFS[FRONTS] = {
    {0, None, 0, 0, 14, {}, {1, 1, 1, 1}, 0, 0, {}, 0, ""},
    {0, None, 0, 0, 22, {}, {0.92f, 0.93f, 0.97f, 1}, 1, 0, {}, 0.6f, "overcast"},
    {260, Rain, 320, 0, 26, {0.72f, 0.82f, 1, 0.55f},
     {0.78f, 0.8f, 0.88f, 1}, 1, 0.15f, {0.6f, 0.66f, 0.76f, 1}, 0.85f, "rain"},
    {320, Rain, 420, 0, 60, {0.75f, 0.84f, 1, 0.65f},
     {0.6f, 0.64f, 0.76f, 1}, 1, 0.3f, {0.55f, 0.6f, 0.7f, 1}, 0.5f, "thunderstorm"},
    {220, Flake, 26, 12, 16, {1, 1, 1, 0.9f}, {0.96f, 0.98f, 1.05f, 1}, 0.7f,
     0.25f, {0.9f, 0.94f, 1, 1}, 0.7f, "snowfall"},
    {0, None, 0, 0, 8, {}, {0.86f, 0.88f, 0.94f, 1}, 0, 1,
     {0.82f, 0.86f, 0.92f, 1}, 0.8f, "fog"},
    {170, Streak, 0.8f, 0, 70, {0.95f, 0.8f, 0.55f, 0.7f},
     {1.02f, 0.9f, 0.74f, 1}, 0, 0.45f, {0.8f, 0.66f, 0.45f, 1}, 0.6f, "sandstorm"},
    {240, Flake, 16, 7, 10, {0.32f, 0.29f, 0.29f, 1},
     {0.86f, 0.76f, 0.72f, 1}, 0, 0.4f, {0.42f, 0.38f, 0.38f, 1}, 0.5f, "ashfall"}};

const uint8_t CHANCES[BIOME_COUNT][FRONTS] = {
    {5, 4, 2, 1, 0, 1, 0, 0}, {4, 4, 3, 1, 0, 2, 0, 0},
    {4, 4, 3, 1, 0, 2, 0, 0}, {4, 4, 2, 1, 0, 3, 0, 0},
    {2, 3, 2, 1, 0, 5, 0, 0}, {3, 3, 0, 0, 6, 1, 0, 0},
    {6, 3, 2, 0, 0, 1, 0, 0}, {2, 3, 3, 1, 0, 5, 0, 0},
    {7, 1, 0, 0, 0, 0, 4, 0}, {2, 3, 6, 3, 0, 2, 0, 0},
    {6, 3, 1, 2, 0, 0, 2, 0}, {2, 0, 0, 0, 0, 1, 0, 7},
    {4, 2, 0, 0, 4, 2, 0, 0}, {2, 2, 2, 0, 0, 6, 0, 0},
    {5, 2, 0, 1, 0, 0, 5, 0}, {3, 4, 1, 0, 5, 2, 0, 0},
    {7, 2, 0, 0, 0, 0, 3, 0}, {2, 3, 6, 2, 0, 3, 0, 0},
    {5, 2, 1, 0, 0, 4, 0, 0}, {3, 3, 3, 1, 0, 4, 0, 0},
    {2, 1, 0, 0, 0, 2, 0, 6}, {5, 4, 2, 1, 0, 1, 0, 0},
    {0, 3, 3, 7, 0, 0, 0, 0}, {6, 1, 0, 0, 0, 0, 5, 0},
    {3, 1, 0, 0, 0, 7, 0, 1}, {7, 3, 0, 0, 0, 1, 0, 0}};

} // namespace

const FrontDef &frontDef(Front f) { return DEFS[int(f)]; }

int frontChance(int biome, Front f) { return CHANCES[biome][int(f)]; }

Front pickFront(int biome, Front current) {
  int total = 0;
  for (int f = 0; f < FRONTS; f++)
    total += CHANCES[biome][f] + (Front(f) == current) * 2;
  int r = rng.irange(0, total - 1);
  for (int f = 0; f < FRONTS; f++)
    if ((r -= CHANCES[biome][f] + (Front(f) == current) * 2) < 0)
      return Front(f);
  return Front::Clear;
}

Front groundLayer(Front f) {
  switch (f) {
  case Front::Rain:
  case Front::Storm:
    return Front::Rain;
  case Front::Snow:
  case Front::Ash:
  case Front::Dust:
    return f;
  default:
    return Front::Clear;
  }
}
