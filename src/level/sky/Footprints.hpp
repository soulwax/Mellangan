#pragma once

#include <array>

class Screen;
class Sky;

class Footprints {
public:
  void tick(const Sky &sky, double px, double py);
  void render(Screen &s, const Sky &sky) const;

private:
  struct Print {
    float x, y, age;
    bool snow;
  };
  std::array<Print, 384> prints{};
  int next = 0;
  bool leftFoot = false;
  float stepX = 0, stepY = 0;
};
