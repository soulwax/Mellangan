#pragma once

#include <vector>

struct SafeZone {
  double x, y;
  float radius;
};

struct RunRules {
  static constexpr int ROLL_OMEN = -2;

  float nightLength = 900, dawnPull = 60, dayLength = 0;
  bool endsAtDawn = true;
  float spawnRate = 1, hordeScale = 1, dayRate = 0;
  bool waves = true, bosses = true, loot = false, town = false, sky = true;
  int omen = ROLL_OMEN;
  std::vector<SafeZone> safe;
  float (*threat)(double x, double y) = nullptr;
  bool (*warded)(double x, double y) = nullptr;
  void (*route)(double x, double y, double px, double py, float speed, double &xw,
                double &yw) = nullptr;
  void (*wall)(double ox, double oy, double &x, double &y) = nullptr;

  bool safeAt(double x, double y) const {
    for (const SafeZone &z : safe) {
      double xd = x - z.x, yd = y - z.y;
      if (xd * xd + yd * yd < double(z.radius) * z.radius)
        return true;
    }
    return false;
  }
};
