#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>

class Upgrade;

enum class StatusKind { Burn, Chill, Poison, Marked, Bleed, Weaken, Fear, Count };

struct StatusEffect {
  float time = 0, power = 0, elapsed = 0;
  Upgrade *source = nullptr;
};

class Status {
public:
  uint32_t immune = 0;
  bool moving = false;
  std::array<float, int(StatusKind::Count)> resist{};

  bool apply(StatusKind kind, float duration, float power,
             Upgrade *source = nullptr) {
    int i = int(kind);
    if (i < 0 || i >= int(StatusKind::Count) || immune & (1u << i) ||
        !std::isfinite(duration) || !std::isfinite(power) || duration <= 0 ||
        power <= 0)
      return false;
    auto &e = effects[i];
    float r = std::clamp(resist[i], 0.0f, 0.75f);
    power = std::min(power * (1 - r), cap(kind)), duration *= 1 - r;
    if (kind == StatusKind::Bleed && e.time > 0) {
      e.power = std::min(cap(kind), e.power + power);
      e.source = source;
    } else if (e.time <= 0 || power > e.power) {
      e.power = power;
      e.source = source;
    }
    e.time = std::max(e.time, std::min(duration, 60.0f));
    live |= 1u << i;
    return true;
  }

  float power(StatusKind kind) const {
    if (!(live >> int(kind) & 1))
      return 0;
    const auto &e = effects[int(kind)];
    return e.time > 0 ? e.power : 0;
  }

  const StatusEffect &effect(StatusKind kind) const { return effects[int(kind)]; }
  void end(StatusKind kind) { effects[int(kind)] = {}, live &= ~(1u << int(kind)); }

  static float cap(StatusKind kind) {
    switch (kind) {
    case StatusKind::Chill:
      return 0.65f;
    case StatusKind::Marked:
    case StatusKind::Fear:
      return 1.0f;
    case StatusKind::Weaken:
      return 0.5f;
    default:
      return 1000.0f;
    }
  }

  static bool ticks(int i) {
    return i == int(StatusKind::Burn) || i == int(StatusKind::Poison) ||
           i == int(StatusKind::Bleed);
  }

  float movement() const { return 1 - power(StatusKind::Chill); }
  float incoming() const { return 1 + power(StatusKind::Marked); }
  float outgoing() const { return 1 - power(StatusKind::Weaken); }
  bool afraid() const { return power(StatusKind::Fear) > 0; }
  void clear() { effects = {}, live = 0; }

  template <class F> void tick(float dt, F &&damage) {
    for (uint32_t bits = live; bits; bits &= bits - 1) {
      int i = std::countr_zero(bits);
      auto &e = effects[i];
      if (e.time <= 0) {
        live &= ~(1u << i);
        continue;
      }
      float step = std::min(dt, e.time);
      e.time = std::max(0.0f, e.time - dt);
      if (ticks(i)) {
        float k = i == int(StatusKind::Bleed) && moving ? 1.6f : 1.0f;
        e.elapsed += step;
        while (e.elapsed + 1e-6f >= 0.5f) {
          e.elapsed = std::max(0.0f, e.elapsed - 0.5f);
          if (!damage(e.power * 0.5f * k, e.source)) {
            clear();
            return;
          }
        }
        if (e.time <= 0 && e.elapsed > 1e-6f)
          if (!damage(e.power * e.elapsed * k, e.source)) {
            clear();
            return;
          }
      }
      if (e.time <= 0)
        e = {}, live &= ~(1u << i);
    }
  }

private:
  std::array<StatusEffect, int(StatusKind::Count)> effects{};
  uint32_t live = 0;
};
