#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

class Upgrade;

enum class StatusKind { Burn, Chill, Poison, Marked, Count };

struct StatusEffect {
  float time = 0, power = 0, elapsed = 0;
  Upgrade *source = nullptr;
};

class Status {
public:
  std::array<StatusEffect, int(StatusKind::Count)> effects{};
  uint32_t immune = 0;

  bool apply(StatusKind kind, float duration, float power,
             Upgrade *source = nullptr) {
    int i = int(kind);
    if (i < 0 || i >= int(StatusKind::Count) || immune & (1u << i) ||
        !std::isfinite(duration) || !std::isfinite(power) || duration <= 0 ||
        power <= 0)
      return false;
    auto &e = effects[i];
    power = std::min(power, kind == StatusKind::Chill ? 0.65f :
                            kind == StatusKind::Marked ? 1.0f : 1000.0f);
    if (e.time <= 0 || power > e.power) {
      e.power = power;
      e.source = source;
    }
    e.time = std::max(e.time, std::min(duration, 60.0f));
    return true;
  }

  float power(StatusKind kind) const {
    const auto &e = effects[int(kind)];
    return e.time > 0 ? e.power : 0;
  }

  float movement() const { return 1 - power(StatusKind::Chill); }
  float incoming() const { return 1 + power(StatusKind::Marked); }
  void clear() { effects = {}; }

  template <class F> void tick(float dt, F &&damage) {
    for (int i = 0; i < int(StatusKind::Count); i++) {
      auto &e = effects[i];
      if (e.time <= 0)
        continue;
      float step = std::min(dt, e.time);
      e.time = std::max(0.0f, e.time - dt);
      if (i == int(StatusKind::Burn) || i == int(StatusKind::Poison)) {
        e.elapsed += step;
        while (e.elapsed + 1e-6f >= 0.5f) {
          e.elapsed = std::max(0.0f, e.elapsed - 0.5f);
          if (!damage(e.power * 0.5f, e.source)) {
            clear();
            return;
          }
        }
        if (e.time <= 0 && e.elapsed > 1e-6f)
          if (!damage(e.power * e.elapsed, e.source)) {
            clear();
            return;
          }
      }
      if (e.time <= 0)
        e = {};
    }
  }
};
