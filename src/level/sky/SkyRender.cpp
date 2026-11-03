#include "level/Sky.hpp"

#include "level/World.hpp"

namespace {

Sprite fleck(Front f, int i) {
  return f == Front::Ash ? atlas::wx::ash(i & 1) : atlas::wx::flake(i & 3);
}

float closeness(float x, float y, float cx, float cy, float reach) {
  float dx = x - cx, dy = y - cy;
  return clampf(1 - std::sqrt(dx * dx + dy * dy) / reach, 0, 1);
}

} // namespace

void Sky::renderCover(Screen &s) const {
  if (cover <= 0.02f || laid == Front::Snow)
    return;
  bool wet = laid == Front::Rain;
  Color tint = wet    ? Color{0.42f, 0.52f, 0.78f, 0.75f * cover}
               : laid == Front::Ash ? Color{0.3f, 0.28f, 0.28f, 0.5f * cover}
                                    : Color{0.9f, 0.75f, 0.5f, 0.4f * cover};
  float grow = 0.7f + 0.4f * cover;
  int x0 = int(std::floor(s.xOffset / 16.0)) - 1,
      y0 = int(std::floor(s.yOffset / 16.0)) - 1;
  for (int y = y0; y <= y0 + s.vh / 16 + 2; y++)
    for (int x = x0; x <= x0 + s.vw / 16 + 2; x++) {
      uint32_t hv = hash2(x * 7 + 1, y * 13 + 5);
      if (hv % (wet ? 2 : 4))
        continue;
      float a = wet ? puddle(x, y) / cover
                    : clampf((0.34f + 0.32f * cover - lowland(x, y)) * 7, 0, 1);
      if (a <= 0)
        continue;
      int v = hv >> 16 & 1;
      float k = grow * (0.8f + 0.4f * a);
      s.sky(wet ? atlas::wx::puddle(v) : atlas::wx::drift(v),
            x * 16 + 8 + int(hv >> 8 & 7) - 4, y * 16 + 8 + int(hv >> 12 & 7) - 4,
            tint.alpha(tint.a * a), k, k);
    }
}

void Sky::renderLanded(Screen &s) const {
  const FrontDef &f = def();
  if (f.fall != Fall::Rain && f.fall != Fall::Flake)
    return;
  bool rain = f.fall == Fall::Rain;
  int n = std::min(active(), int(drops.size()));
  for (int i = 0; i < n; i++) {
    const Drop &d = drops[i];
    if (!d.landed || d.idle)
      continue;
    int dx = int(std::floor(d.x / 16)), dy = int(std::floor(d.y / 16));
    bool pool = rain && (puddle(dx, dy) > 0.25f || poolAt(dx, dy) ||
                         world::isWater(dx, dy));
    if (rain && d.age < 0.1f)
      s.sky(atlas::wx::splash(d.age < 0.05f ? 0 : 1), d.x, d.y - 3,
            f.tint.alpha((pool ? 0.7f : 0.45f) * d.depth));
    else if (rain && !pool)
      continue;
    else if (rain) {
      int frame = std::min(3, int((d.age - 0.1f) / 0.26f * 4));
      s.sky(atlas::wx::ripple(frame), d.x, d.y, f.tint.alpha(0.6f * d.depth));
    } else
      s.sky(fleck(front, i), d.x, d.y, f.tint.alpha(f.tint.a * (1 - d.age / 0.8f)));
  }
}

void Sky::renderGround(Screen &s) const {
  if (laid == Front::Rain && cover > 0.02f)
    s.rect(s.xOffset, s.yOffset, float(s.vw), float(s.vh),
           {0.03f, 0.05f, 0.12f, 0.16f * cover});
  prints.render(s, *this);
  renderCover(s);
  renderLanded(s);
}

void Sky::render(Screen &s, Color air) const {
  const FrontDef &f = def();
  if (f.clouds > 0)
    for (const Bank &b : clouds)
      s.sky(atlas::wx::Cloud, b.x, b.y,
            Color{0.02f, 0.03f, 0.08f,
                  0.42f * f.clouds * amount * b.alpha *
                      (0.45f + 0.75f * local(b.x, b.y))},
            b.scale, b.scale * b.aspect, b.flip ? PI : 0);
  int n = std::min(active(), int(drops.size()));
  for (int i = 0; i < n; i++) {
    const Drop &d = drops[i];
    if (d.landed || d.idle)
      continue;
    switch (f.fall) {
    case Fall::Rain:
      s.sky(atlas::wx::Drop, d.x, d.y - d.z,
            f.tint.alpha(f.tint.a * (0.4f + 0.6f * d.depth)), 1,
            1.1f + 1.8f * d.depth,
            std::atan2(-windX * 1.2f * d.gust, std::max(1.0f, d.speed)));
      break;
    case Fall::Flake:
      s.sky(fleck(front, i), d.x, d.y - d.z,
            f.tint.alpha(f.tint.a * (0.5f + 0.5f * d.depth)),
            d.depth > 0.96f ? 2.0f : 1.0f, d.depth > 0.96f ? 2.0f : 1.0f);
      break;
    case Fall::Streak:
      s.sky(atlas::wx::Drop, d.x, d.y, f.tint.alpha(f.tint.a * std::min(1.0f, d.z * 3)),
            1, 1.5f + (i & 3), float(TAU / 4));
      break;
    default:
      break;
    }
  }
  if (f.haze > 0)
    for (const Bank &b : banks)
      s.sky(b.alt ? atlas::wx::FogB : atlas::wx::FogA, b.x, b.y,
            Color{f.mist.r * air.r, f.mist.g * air.g, f.mist.b * air.b,
                  0.5f * f.haze * amount * b.alpha * local(b.x, b.y)},
            b.scale, b.scale * b.aspect, b.flip ? PI : 0);
}

void Sky::renderLight(Screen &s) const {
  float flash = thunder.flash;
  int x0 = int(std::floor((cx - 96) / 16)), y0 = int(std::floor((cy - 72) / 16));
  for (int y = y0; y <= y0 + 9; y++)
    for (int x = x0; x <= x0 + 12; x++) {
      float a = puddle(x, y);
      if (poolAt(x, y) || (world::isWater(x, y) && !world::isIce(x, y))) {
        uint32_t g = hash2(x * 5 + 2, y * 11 + 7);
        if (g % 3)
          continue;
        float gx = x * 16 + 2 + float(g >> 8 & 11), gy = y * 16 + 2 + float(g >> 12 & 11);
        float shine = (closeness(gx, gy, cx, cy, 120) * 0.35f + flash * 0.9f) *
                      (0.7f + 0.3f * std::sin(ox * 0.05f + float(g & 63)));
        if (shine > 0.04f)
          s.glow(gx, gy, 12, Color{0.4f, 0.5f, 0.72f, 1} * shine);
        continue;
      }
      if (a < 0.3f)
        continue;
      uint32_t hv = hash2(x * 7 + 1, y * 13 + 5);
      float px = float(x * 16 + 8 + int(hv >> 8 & 7) - 4),
            py = float(y * 16 + 8 + int(hv >> 12 & 7) - 4);
      float shine = a * (closeness(px, py, cx, cy, 110) * 0.5f + flash * 0.8f);
      if (shine > 0.04f)
        s.glow(px, py - 1, 14, Color{0.45f, 0.55f, 0.75f, 1} * shine);
    }
}
