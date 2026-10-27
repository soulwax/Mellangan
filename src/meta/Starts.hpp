#pragma once

#include "meta/Codex.hpp"

struct Start {
  const char *weapon, *need;
  bool (*met)(const Codex &);
};

constexpr int START_COUNT = 6;
extern const Start STARTS[START_COUNT];
