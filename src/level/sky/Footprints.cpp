#include "level/sky/Footprints.hpp"

#include "Math.hpp"
#include "level/Sky.hpp"

void Footprints::tick(const Sky &sky, double px, double py) {
  for (Print &pr : prints)
    pr.age += DT;
  float sx = float(px) - stepX, sy = float(py) - stepY;
  if (sx * sx + sy * sy <= 36)
    return;
  int tx = int(std::floor(px / 16)), ty = int(std::floor(py / 16));
  bool snow = sky.snowy(tx, ty);
  if (snow || (sky.laid == Front::Rain && sky.cover > 0.35f)) {
    float d = std::sqrt(sx * sx + sy * sy), side = leftFoot ? 1.5f : -1.5f;
    prints[next] = {float(px) - sy / d * side, float(py) + sx / d * side, 0, snow};
    next = (next + 1) % int(prints.size());
    leftFoot = !leftFoot;
  }
  stepX = float(px), stepY = float(py);
}

void Footprints::render(Screen &s, const Sky &sky) const {
  for (const Print &pr : prints) {
    if (pr.age <= 0 || (!pr.snow && pr.age >= 9) || !s.visible(pr.x, pr.y, 4))
      continue;
    int tx = int(std::floor(pr.x / 16)), ty = int(std::floor(pr.y / 16));
    if (pr.snow && !sky.snowy(tx, ty))
      continue;
    float fade = pr.snow ? 1 : 1 - pr.age / 9;
    s.rect(std::floor(pr.x) - 1, std::floor(pr.y) - 1, 2, 2,
           pr.snow ? Color{0.36f, 0.42f, 0.62f, 0.85f * fade}
                   : Color{0.1f, 0.08f, 0.07f, 0.5f * fade * sky.cover});
  }
}
