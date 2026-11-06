#include "meta/Heat.hpp"

#include <algorithm>

#include "level/RunRules.hpp"
#include "meta/Balance.hpp"

const Rung HEAT[HEAT_MAX] = {
    {"THICK HIDES", "foes have 25% more health",
     [](Balance &b, RunRules &) { b.enemyHp *= 1.25f; }},
    {"HUNGRY DARK", "foes move 10% faster",
     [](Balance &b, RunRules &) { b.enemySpeed *= 1.1f; }},
    {"SHARP CLAWS", "foes hit 25% harder",
     [](Balance &b, RunRules &) { b.enemyDamage *= 1.25f; }},
    {"CHOSEN ONES", "twice as many elites",
     [](Balance &b, RunRules &) { b.eliteBase *= 2; }},
    {"DIM SHRINES", "a lit shrine pulls dawn half as far",
     [](Balance &, RunRules &r) { r.dawnPull *= 0.5f; }},
    {"TEEMING", "foes come 15% faster and 30% more crowd the night",
     [](Balance &b, RunRules &) { b.spawnInterval *= 0.85f, b.enemyCap *= 1.3f; }},
    {"FRAIL", "you have 20% less health",
     [](Balance &b, RunRules &) { b.playerHp *= 0.8f; }},
    {"STARVED", "20% less experience",
     [](Balance &b, RunRules &) { b.xp *= 0.8f; }},
    {"THE TIDE", "hordes are half again as large",
     [](Balance &b, RunRules &) { b.hordeBase *= 1.5f, b.hordeMax *= 1.5f; }},
    {"LONG NIGHT", "the night lasts 20% longer",
     [](Balance &, RunRules &r) { r.nightLength *= 1.2f; }}};

void applyHeat(int heat, Balance &b, RunRules &r) {
  for (int i = 0; i < std::clamp(heat, 0, HEAT_MAX); i++)
    HEAT[i].apply(b, r);
}

std::string heatNames(int heat) {
  std::string out;
  for (int i = 0; i < std::clamp(heat, 0, HEAT_MAX); i++)
    out += (out.empty() ? "" : "   ") + std::string(HEAT[i].name);
  return out;
}
