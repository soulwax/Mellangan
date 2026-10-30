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
  void render(Screen &s) override;
  float occlusion(float &width) const override;
  void renderShadow(Screen &s) override;
};
