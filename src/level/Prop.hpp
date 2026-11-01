#pragma once

#include "SpriteObject.hpp"

class Prop : public SpriteObject {
public:
  struct Part {
    Sprite sprite;
    int yo;
  };
  Part parts[4]{};
  int count = 0;
  float shadow = 0, solid = 0, lightSize = 80;
  Color light{0, 0, 0, 0};
  int lightYo = -10, sway = 0;
  bool wisp = true;

  void add(const Sprite &s, int yo = 0) { parts[count++] = {s, yo}; }
  bool body(float shade, float collide) {
    shadow = shade, solid = collide;
    return true;
  }
  bool piece(const Sprite &s, float shade, float collide) {
    add(s);
    return body(shade, collide);
  }
  void lamp(Color c, float size = 80, int yo = -10) {
    light = c, lightSize = size, lightYo = yo;
  }
  void fire(Color c, float size, int yo) { lamp(c, size, yo), wisp = false; }
  void render(Screen &s) override;
  float occlusion(float &width) const override;
  void renderShadow(Screen &s) override;
};
