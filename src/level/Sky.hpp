#pragma once

#include <array>

#include "gfx/Screen.hpp"

class Level;

enum class Front { Clear, Clouds, Rain, Storm, Snow, Fog, Dust, Ash, Count };

class Sky {
public:
  Front front = Front::Clear, next = Front::Clear;
  float amount = 0, flash = 0, windX = 12, windY = 0, cover = 0;
  Front laid = Front::Clear;

  void tick(Level &level);
  void force(Front f);
  void renderGround(Screen &s) const;
  void renderLight(Screen &s) const;
  void render(Screen &s, Color air) const;
  Color shade() const;
  const char *name() const;
  float rainfall() const;
  float rough() const;
  bool snowAt(int x, int y) const;
  bool snowy(int x, int y) const;
  float gale() const;
  float local(float x, float y) const;
  static float lowland(int x, int y);
  float puddle(int x, int y) const;
  float flood() const;
  bool poolAt(int x, int y) const;
  int sway(double x, double y, float time) const;

private:
  struct Drop {
    float x, y, z, speed, age, depth, phase, gust;
    bool landed, idle;
  };
  struct Bank {
    float x, y, scale, aspect, alpha, speed;
    bool alt, flip;
  };
  struct Print {
    float x, y, age;
    bool snow;
  };
  std::array<Drop, 320> drops{};
  std::array<Bank, 12> clouds{};
  std::array<Bank, 14> banks{};
  std::array<Print, 384> prints{};
  int printNext = 0;
  bool leftFoot = false;
  float stepX = 0, stepY = 0, thunderIn = 0, thunderVol = 0, thunderPitch = 1;
  float timer = 25, strikeTime = 4, flicker = 0, cx = 0, cy = 0, w = 560,
        h = 340, ox = 0, oy = 0, heading = 0, surge = 1;
  bool seeded = false, pinned = false;
  Front shown = Front::Clear;

  int active() const;
  float gustAt(float x, float y) const;
  void respawn(Drop &d, bool fresh) const;
  void place(Bank &b, const Bank *near) const;
};
