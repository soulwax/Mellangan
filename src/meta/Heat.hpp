#pragma once

#include <string>

struct Balance;
struct RunRules;

struct Rung {
  const char *name, *text;
  void (*apply)(Balance &b, RunRules &r);
};

constexpr int HEAT_MAX = 10;
extern const Rung HEAT[HEAT_MAX];

void applyHeat(int heat, Balance &b, RunRules &r);
std::string heatNames(int heat);
