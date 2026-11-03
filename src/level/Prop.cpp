#include "level/Prop.hpp"

#include "level/LightField.hpp"

void Prop::render(Screen &s) {
  bool tinted = tone != 1 || warm != 0;
  if (tinted)
    s.color({tone * (1 + warm), tone, tone * (1 - warm), 1});
  for (int i = 0; i < count; i++)
    s.sprite(parts[i].sprite, xp + std::clamp(-parts[i].yo / 16, 0, 2) * sway,
             yp + parts[i].yo, 8, 16, flip ? -1.0f : 1.0f, 1);
  if (tinted)
    s.color(col::white);
}

float Prop::occlusion(float &width) const {
  return shadow > 0 ? (width = shadow, 4 + 12.0f * count) : (width = 0, 0.0f);
}

void Prop::renderShadow(Screen &s) {
  if (shadow > 0)
    s.sprite(atlas::Shadow, xp, yp - 1, 8, 4, shadow, 1.2f);
}

void Prop::shine(Screen &s, LightField &lights, float time) const {
  if (light.a <= 0)
    return;
  if (wisp) {
    float f = 0.8f + 0.2f * float(std::sin(time * 2 + x * 0.37));
    s.glow(x, y + lightYo + std::sin(time * 1.7 + y) * 2, lightSize, light * f);
    return;
  }
  float f = flicker(time, float(x * 0.13 + y * 0.07));
  s.glow(x, y + lightYo, lightSize * (0.95f + 0.05f * f), light * f);
  lights.add(x, y + lightYo + 6, lightSize * 0.9f, f);
}
