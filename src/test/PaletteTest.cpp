#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "gfx/Palette.hpp"
#include "item/loot/Item.hpp"

namespace {
const float DEUTAN[3][3] = {{0.367322f, 0.860646f, -0.227968f},
                            {0.280085f, 0.672501f, 0.047413f},
                            {-0.011820f, 0.042940f, 0.968881f}};
const float PROTAN[3][3] = {{0.152286f, 1.052583f, -0.204868f},
                            {0.114503f, 0.786281f, 0.099216f},
                            {-0.003882f, -0.048116f, 1.051998f}};

float linear(float c) { return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f); }
float encode(float c) { return c <= 0.0031308f ? 12.92f * c : 1.055f * std::pow(c, 1 / 2.4f) - 0.055f; }

Color seen(const float (&m)[3][3], Color c) {
  float l[3] = {linear(c.r), linear(c.g), linear(c.b)}, o[3];
  for (int i = 0; i < 3; i++)
    o[i] = encode(std::clamp(m[i][0] * l[0] + m[i][1] * l[1] + m[i][2] * l[2], 0.0f, 1.0f));
  return {o[0], o[1], o[2], 1};
}

float apart(const float (&m)[3][3], Color a, Color b) {
  Color x = seen(m, a), y = seen(m, b);
  return std::sqrt((x.r - y.r) * (x.r - y.r) + (x.g - y.g) * (x.g - y.g) + (x.b - y.b) * (x.b - y.b));
}

std::vector<Color> rarities() {
  std::vector<Color> out;
  for (Rarity r : {Rarity::Magic, Rarity::Rare, Rarity::Unique, Rarity::Crafted}) {
    Item item;
    item.rarity = r;
    out.push_back(item.color());
  }
  Item night;
  night.curse = 0;
  out.push_back(night.color());
  return out;
}

float closest(const float (&m)[3][3]) {
  std::vector<Color> c = rarities();
  float best = 9;
  for (size_t i = 0; i < c.size(); i++)
    for (size_t j = i + 1; j < c.size(); j++)
      best = std::min(best, apart(m, c[i], c[j]));
  return best;
}
} // namespace

void testPalette() {
  constexpr Color GRASS{0.52f, 0.78f, 0.41f, 1};
  float before[2][3], after[2][3];
  for (int safe = 0; safe < 2; safe++) {
    palette::safe = safe;
    float (&out)[2][3] = safe ? after : before;
    int i = 0;
    for (const auto *m : {&DEUTAN, &PROTAN}) {
      out[i][0] = closest(*m);
      out[i][1] = apart(*m, palette::hurt(), palette::heal());
      out[i][2] = apart(*m, palette::warning(1), GRASS);
      i++;
    }
  }
  palette::safe = false;
  for (int i = 0; i < 2; i++)
    for (int k = 0; k < 3; k++)
      assert(after[i][k] > before[i][k]);
  assert(after[0][0] > 0.25f && after[1][0] > 0.25f);
  std::cout << "[TEST] Palette: under simulated deuteranopia and protanopia the colourblind palette "
               "pulls rarities apart ("
            << int(before[0][0] * 100) << "/" << int(before[1][0] * 100) << " -> "
            << int(after[0][0] * 100) << "/" << int(after[1][0] * 100)
            << "), and damage from healing and warnings from grass: PASS" << std::endl;
}
