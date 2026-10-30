#include "level/Sky.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "level/Noise.h"
#include "level/World.hpp"

namespace {
enum Fall { None, Rain, Flake, Streak };

struct FrontDef {
  int drops;
  Fall fall;
  float speed, sway, wind;
  Color tint, shade;
  float clouds, haze;
  Color mist;
  float patchy;
};

constexpr int FRONTS = int(Front::Count);

const FrontDef DEFS[FRONTS] = {
    {0, None, 0, 0, 14, {}, {1, 1, 1, 1}, 0, 0, {}, 0},
    {0, None, 0, 0, 22, {}, {0.92f, 0.93f, 0.97f, 1}, 1, 0, {}, 0.6f},
    {260, Rain, 320, 0, 26, {0.72f, 0.82f, 1, 0.55f},
     {0.78f, 0.8f, 0.88f, 1}, 1, 0.15f, {0.6f, 0.66f, 0.76f, 1}, 0.85f},
    {320, Rain, 420, 0, 60, {0.75f, 0.84f, 1, 0.65f},
     {0.6f, 0.64f, 0.76f, 1}, 1, 0.3f, {0.55f, 0.6f, 0.7f, 1}, 0.5f},
    {220, Flake, 26, 12, 16, {1, 1, 1, 0.9f}, {0.96f, 0.98f, 1.05f, 1}, 0.7f,
     0.25f, {0.9f, 0.94f, 1, 1}, 0.7f},
    {0, None, 0, 0, 8, {}, {0.86f, 0.88f, 0.94f, 1}, 0, 1,
     {0.82f, 0.86f, 0.92f, 1}, 0.8f},
    {170, Streak, 0.8f, 0, 70, {0.95f, 0.8f, 0.55f, 0.7f},
     {1.02f, 0.9f, 0.74f, 1}, 0, 0.45f, {0.8f, 0.66f, 0.45f, 1}, 0.6f},
    {240, Flake, 16, 7, 10, {0.32f, 0.29f, 0.29f, 1},
     {0.86f, 0.76f, 0.72f, 1}, 0, 0.4f, {0.42f, 0.38f, 0.38f, 1}, 0.5f}};

const uint8_t CHANCES[BIOME_COUNT][FRONTS] = {
    {5, 4, 2, 1, 0, 1, 0, 0}, {4, 4, 3, 1, 0, 2, 0, 0},
    {4, 4, 3, 1, 0, 2, 0, 0}, {4, 4, 2, 1, 0, 3, 0, 0},
    {2, 3, 2, 1, 0, 5, 0, 0}, {3, 3, 0, 0, 6, 1, 0, 0},
    {6, 3, 2, 0, 0, 1, 0, 0}, {2, 3, 3, 1, 0, 5, 0, 0},
    {7, 1, 0, 0, 0, 0, 4, 0}, {2, 3, 6, 3, 0, 2, 0, 0},
    {6, 3, 1, 2, 0, 0, 2, 0}, {2, 0, 0, 0, 0, 1, 0, 7},
    {4, 2, 0, 0, 4, 2, 0, 0}, {2, 2, 2, 0, 0, 6, 0, 0},
    {5, 2, 0, 1, 0, 0, 5, 0}, {3, 4, 1, 0, 5, 2, 0, 0},
    {7, 2, 0, 0, 0, 0, 3, 0}, {2, 3, 6, 2, 0, 3, 0, 0},
    {5, 2, 1, 0, 0, 4, 0, 0}, {3, 3, 3, 1, 0, 4, 0, 0},
    {2, 1, 0, 0, 0, 2, 0, 6}, {5, 4, 2, 1, 0, 1, 0, 0},
    {0, 3, 3, 7, 0, 0, 0, 0}, {6, 1, 0, 0, 0, 0, 5, 0},
    {3, 1, 0, 0, 0, 7, 0, 1}, {7, 3, 0, 0, 0, 1, 0, 0}};

Front pick(int biome, Front current) {
  int total = 0;
  for (int f = 0; f < FRONTS; f++)
    total += CHANCES[biome][f] + (Front(f) == current) * 2;
  int r = rng.irange(0, total - 1);
  for (int f = 0; f < FRONTS; f++)
    if ((r -= CHANCES[biome][f] + (Front(f) == current) * 2) < 0)
      return Front(f);
  return Front::Clear;
}

Color mix(Color a, Color b, float k) {
  return {lerpf(a.r, b.r, k), lerpf(a.g, b.g, k), lerpf(a.b, b.b, k), 1};
}

float wrap(float v, float centre, float span) {
  while (v - centre > span / 2)
    v -= span;
  while (v - centre < -span / 2)
    v += span;
  return v;
}
Front layer(Front f) {
  switch (f) {
  case Front::Rain:
  case Front::Storm:
    return Front::Rain;
  case Front::Snow:
  case Front::Ash:
  case Front::Dust:
    return f;
  default:
    return Front::Clear;
  }
}
} // namespace

void Sky::force(Front f) {
  front = next = f, amount = 1, timer = 90, pinned = true;
  laid = layer(f), cover = laid == Front::Clear ? 0.0f : 0.5f;
}

int Sky::active() const {
  return int(DEFS[int(front)].drops * amount * surge);
}

float Sky::local(float x, float y) const {
  float n = noise_fbm((x - ox) * 0.0032f, (y - oy) * 0.0032f, 2, 909);
  float cells = clampf(0.45f + n * 2.2f, 0, 1);
  cells = cells * cells * (3 - 2 * cells);
  return lerpf(1, cells, DEFS[int(front)].patchy);
}

float Sky::lowland(int x, int y) {
  return clampf(0.5f + noise_fbm(x * 0.07f + 13.1f, y * 0.07f - 4.7f, 2, 917),
                0, 1);
}

float Sky::puddle(int x, int y) const {
  if (laid != Front::Rain || cover <= 0.02f || hash2(x * 7 + 1, y * 13 + 5) % 2)
    return 0;
  return clampf((lowland(x, y) - 0.66f + 0.32f * cover) * 7, 0, 1) * cover;
}

float Sky::flood() const {
  return laid == Front::Rain ? clampf((cover - 0.4f) / 0.6f, 0, 1) : 0;
}

bool Sky::poolAt(int x, int y) const {
  float depth = flood();
  return depth > 0 && lowland(x, y) > 0.9f - 0.34f * depth;
}

int Sky::sway(double x, double y, float time) const {
  float g = gustAt(float(x), float(y));
  float bend = windX / 40 * g *
               (0.65f + 0.35f * std::sin(time * 2.3f + float(x) * 0.05f));
  return int(std::lround(clampf(bend, -2, 2)));
}

float Sky::gustAt(float x, float y) const {
  float g = noise_perlin((x - ox * 2.5f) * 0.011f, (y - oy * 2.5f) * 0.011f, 911);
  return clampf(0.75f + g * 1.3f, 0.25f, 2);
}

void Sky::respawn(Drop &d, bool fresh) const {
  const FrontDef &f = DEFS[int(front)];
  d.x = cx + rng.range(-w / 2, w / 2);
  d.y = cy + rng.range(-h / 2, h / 2);
  d.depth = rng.range(f.fall == Rain ? 0.5f : 0.7f, 1);
  d.speed = f.speed * rng.range(0.9f, 1.1f) * (f.fall == Streak ? 1 : d.depth);
  d.phase = rng.unit(), d.gust = gustAt(d.x, d.y);
  d.age = rng.range(0, 6), d.landed = false;
  d.idle = rng.unit() > local(d.x, d.y);
  float top = f.fall == Rain ? 240 : f.fall == Flake ? 190 : 1;
  d.z = d.idle ? rng.range(0.15f, 0.6f)
               : top * (fresh ? rng.unit() : rng.range(0.6f, 1));
}

void Sky::place(Bank &b, const Bank *near) const {
  if (near) {
    b.x = near->x + rng.range(-80, 80), b.y = near->y + rng.range(-45, 45);
    b.speed = near->speed;
  } else {
    b.x = cx + rng.range(-w / 2 - 220, w / 2 + 220);
    b.y = cy + rng.range(-h / 2 - 160, h / 2 + 160);
    b.speed = rng.range(0.6f, 1.3f);
  }
  b.scale = rng.range(near ? 1.4f : 2.2f, near ? 2.6f : 3.8f);
  b.aspect = rng.range(0.45f, 0.85f), b.alpha = rng.range(0.55f, 1);
  b.alt = rng.chance(0.5f), b.flip = rng.chance(0.5f);
}

void Sky::tick(Level &level) {
  Player &p = *level.player;
  cx = float(p.x), cy = float(p.y);
  int biome = world::biomeAt(int(std::floor(p.x / 16)), int(std::floor(p.y / 16)));
  if (!seeded) {
    seeded = true;
    for (size_t i = 0; i < clouds.size(); i++)
      place(clouds[i], i % 3 ? &clouds[i - i % 3] : nullptr);
    for (size_t i = 0; i < banks.size(); i++)
      place(banks[i], i % 2 ? &banks[i - 1] : nullptr);
  }
  if ((timer -= DT) <= 0 ||
      (!pinned && next == front && !CHANCES[biome][int(front)])) {
    timer = rng.range(45, 100), pinned = false;
    next = pick(biome, front);
    if (!CHANCES[biome][int(next)])
      next = Front::Clear;
  }
  int before = active();
  if (next != front) {
    if ((amount -= DT / 4) <= 0)
      amount = 0, front = next,
      heading = (rng.chance(0.5f) ? 0 : PI) + rng.range(-0.5f, 0.5f);
  } else
    amount = std::min(1.0f, amount + DT / 6);
  const FrontDef &f = DEFS[int(front)];
  float t = level.time;
  float gust = 0.7f + 0.3f * std::sin(t * 0.07f) + 0.15f * std::sin(t * 0.61f);
  float angle =
      heading + 0.35f * std::sin(t * 0.021f) + 0.15f * std::sin(t * 0.093f);
  windX = lerpf(windX, f.wind * gust * std::cos(angle), DT * 0.5f);
  windY = lerpf(windY, f.wind * gust * std::sin(angle) * 0.6f, DT * 0.5f);
  ox += windX * 0.6f * DT, oy += windY * 0.6f * DT;
  surge = 0.75f + 0.25f * std::sin(t * 0.083f + heading * 3);
  int n = std::min(active(), int(drops.size()));
  if (level.game.reducedParticles)
    n = n * 2 / 3;
  for (int i = shown == front ? before : 0; i < n; i++)
    respawn(drops[i], true);
  shown = front;
  for (int i = 0; i < n; i++) {
    Drop &d = drops[i];
    d.age += DT;
    if (d.idle) {
      if ((d.z -= DT) <= 0)
        respawn(d, false);
      continue;
    }
    d.gust = lerpf(d.gust, gustAt(d.x, d.y), DT * 3);
    float push = d.gust * (f.fall == Rain ? 1.2f : f.fall == Flake ? 0.6f : 5);
    if (f.fall == Streak) {
      d.x += windX * push * DT, d.y += windY * push * DT;
      if ((d.z -= DT) <= 0)
        respawn(d, false);
    } else if (d.landed) {
      if (d.age > (f.fall == Rain ? 0.36f : 0.8f))
        respawn(d, false);
    } else {
      float flutter = f.sway * d.depth;
      d.x += (windX * push +
              std::sin(d.age * (1.3f + d.phase) + d.phase * 6) * flutter) *
             DT;
      d.y += (windY * push +
              std::cos(d.age * 0.9f + d.phase * 4) * flutter * 0.3f) *
             DT;
      if ((d.z -= d.speed * DT) <= 0)
        d.z = 0, d.landed = true, d.age = 0;
    }
    d.x = wrap(d.x, cx, w), d.y = wrap(d.y, cy, h);
  }
  for (Bank &b : clouds) {
    b.x = wrap(b.x + (windX * 0.8f + 4) * b.speed * DT, cx, w + 440);
    b.y = wrap(b.y + windY * 0.8f * b.speed * DT, cy, h + 320);
  }
  for (Bank &b : banks) {
    b.x = wrap(b.x + windX * 0.4f * b.speed * DT, cx, w + 360);
    b.y = wrap(b.y + windY * 0.4f * b.speed * DT, cy, h + 260);
  }
  Front want = layer(front);
  if (want != Front::Clear && (laid == want || cover <= 0))
    laid = want, cover = std::min(amount, cover + DT * amount / (front == Front::Storm ? 28 : 50));
  else
    cover = std::max(0.0f, cover - DT / (CHANCES[biome][int(laid)] ? 70 : 6));
  flash = std::max(0.0f, flash - DT * 3);
  if (flicker > 0 && (flicker -= DT) <= 0)
    flash = std::max(flash, 0.75f);
  for (Print &pr : prints)
    pr.age += DT;
  float sx = float(p.x) - stepX, sy = float(p.y) - stepY;
  if (sx * sx + sy * sy > 36) {
    int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16));
    bool snow = snowy(tx, ty);
    if (snow || (laid == Front::Rain && cover > 0.35f)) {
      float d = std::sqrt(sx * sx + sy * sy), side = leftFoot ? 1.5f : -1.5f;
      prints[printNext] = {float(p.x) - sy / d * side, float(p.y) + sx / d * side,
                           0, snow};
      printNext = (printNext + 1) % int(prints.size());
      leftFoot = !leftFoot;
    }
    stepX = float(p.x), stepY = float(p.y);
  }
  if (thunderIn > 0 && (thunderIn -= DT) <= 0)
    Sound::thunder.play(thunderVol, 0.04f, thunderPitch);
  if (front != Front::Storm || amount < 0.6f || p.dead())
    return;
  if (rng.chance(DT / 7)) {
    flash = std::max(flash, 0.3f);
    if (thunderIn <= 0)
      thunderIn = rng.range(1.5f, 3.5f), thunderVol = 0.1f,
      thunderPitch = rng.range(0.5f, 0.65f);
  }
  if ((strikeTime -= DT) <= 0) {
    strikeTime = rng.range(3, 8);
    float d = level.strike();
    if (d >= 0) {
      flash = 1, flicker = 0.13f;
      thunderIn = 0.1f + d / 340, thunderVol = clampf(0.34f - d / 1100, 0.12f, 0.34f);
      thunderPitch = clampf(0.95f - d / 1400, 0.65f, 0.95f);
    }
  }
}

void Sky::renderGround(Screen &s) const {
  if (laid == Front::Rain && cover > 0.02f)
    s.rect(s.xOffset, s.yOffset, float(s.vw), float(s.vh),
           {0.03f, 0.05f, 0.12f, 0.16f * cover});
  for (const Print &pr : prints) {
    if (pr.age <= 0 || (!pr.snow && pr.age >= 9) || !s.visible(pr.x, pr.y, 4))
      continue;
    int tx = int(std::floor(pr.x / 16)), ty = int(std::floor(pr.y / 16));
    if (pr.snow && !snowy(tx, ty))
      continue;
    float fade = pr.snow ? 1 : 1 - pr.age / 9;
    s.rect(std::floor(pr.x) - 1, std::floor(pr.y) - 1, 2, 2,
           pr.snow ? Color{0.36f, 0.42f, 0.62f, 0.85f * fade}
                   : Color{0.1f, 0.08f, 0.07f, 0.5f * fade * cover});
  }
  if (cover > 0.02f && laid != Front::Snow) {
    bool wet = laid == Front::Rain;
    Color tint = wet    ? Color{0.42f, 0.52f, 0.78f, 0.75f * cover}
                 : laid == Front::Ash ? Color{0.3f, 0.28f, 0.28f, 0.5f * cover}
                                      : Color{0.9f, 0.75f, 0.5f, 0.4f * cover};
    float grow = 0.7f + 0.4f * cover;
    int x0 = int(std::floor(s.xOffset / 16.0)) - 1,
        y0 = int(std::floor(s.yOffset / 16.0)) - 1;
    for (int y = y0; y <= y0 + s.vh / 16 + 2; y++)
      for (int x = x0; x <= x0 + s.vw / 16 + 2; x++) {
        uint32_t h = hash2(x * 7 + 1, y * 13 + 5);
        if (h % (wet ? 2 : 4))
          continue;
        float a = wet ? puddle(x, y) / cover
                      : clampf((0.34f + 0.32f * cover - lowland(x, y)) * 7, 0, 1);
        if (a <= 0)
          continue;
        int v = h >> 16 & 1;
        float k = grow * (0.8f + 0.4f * a);
        s.sky(wet ? atlas::wx::puddle(v) : atlas::wx::drift(v),
              x * 16 + 8 + int(h >> 8 & 7) - 4, y * 16 + 8 + int(h >> 12 & 7) - 4,
              tint.alpha(tint.a * a), k, k);
      }
  }
  const FrontDef &f = DEFS[int(front)];
  if (f.fall != Rain && f.fall != Flake)
    return;
  int n = std::min(active(), int(drops.size()));
  for (int i = 0; i < n; i++) {
    const Drop &d = drops[i];
    if (!d.landed || d.idle)
      continue;
    int dx = int(std::floor(d.x / 16)), dy = int(std::floor(d.y / 16));
    bool pool = f.fall == Rain && (puddle(dx, dy) > 0.25f || poolAt(dx, dy));
    if (f.fall == Rain && d.age < 0.1f)
      s.sky(atlas::wx::splash(d.age < 0.05f ? 0 : 1), d.x, d.y - 3,
            f.tint.alpha((pool ? 0.7f : 0.45f) * d.depth));
    else if (f.fall == Rain && !pool)
      continue;
    else if (f.fall == Rain) {
      int frame = std::min(3, int((d.age - 0.1f) / 0.26f * 4));
      s.sky(atlas::wx::ripple(frame), d.x, d.y, f.tint.alpha(0.6f * d.depth));
    } else {
      s.sky(front == Front::Ash ? atlas::wx::ash(i & 1) : atlas::wx::flake(i & 3),
            d.x, d.y,
            f.tint.alpha(f.tint.a * (1 - d.age / 0.8f)));
    }
  }
}

void Sky::render(Screen &s, Color air) const {
  const FrontDef &f = DEFS[int(front)];
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
    case Rain:
      s.sky(atlas::wx::Drop, d.x, d.y - d.z,
            f.tint.alpha(f.tint.a * (0.4f + 0.6f * d.depth)), 1,
            1.1f + 1.8f * d.depth,
            std::atan2(-windX * 1.2f * d.gust, std::max(1.0f, d.speed)));
      break;
    case Flake:
      s.sky(front == Front::Ash ? atlas::wx::ash(i & 1) : atlas::wx::flake(i & 3),
            d.x, d.y - d.z, f.tint.alpha(f.tint.a * (0.5f + 0.5f * d.depth)),
            d.depth > 0.96f ? 2.0f : 1.0f, d.depth > 0.96f ? 2.0f : 1.0f);
      break;
    case Streak:
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

Color Sky::shade() const {
  Color c = mix({1, 1, 1, 1}, DEFS[int(front)].shade, amount);
  return {c.r + flash * 0.6f, c.g + flash * 0.65f, c.b + flash * 0.8f, 1};
}

const char *Sky::name() const {
  if (amount < 0.3f)
    return "";
  static const char *const NAMES[FRONTS] = {
      "", "overcast", "rain", "thunderstorm", "snowfall", "fog", "sandstorm", "ashfall"};
  return front == Front::Rain && amount < 0.7f ? "drizzle" : NAMES[int(front)];
}

float Sky::rainfall() const {
  float k = front == Front::Storm ? 1 : front == Front::Rain ? 0.65f : 0;
  return k * amount * surge * (0.4f + 0.6f * local(cx, cy));
}

float Sky::gale() const {
  float speed = std::sqrt(windX * windX + windY * windY);
  float k = front == Front::Storm || front == Front::Dust ? 1 : 0.45f;
  return clampf((speed - 10) / 60, 0, 1) * k * std::max(amount, 0.3f);
}

float Sky::rough() const {
  return front == Front::Clear || front == Front::Clouds || front == Front::Fog
             ? 0
             : amount;
}

bool Sky::snowy(int x, int y) const {
  Structure site;
  return snowAt(x, y) || (BIOMES[world::biomeAt(x, y)]->winter() &&
                          !world::isDirt(x, y) && !world::structureAt(x, y, site));
}

bool Sky::snowAt(int x, int y) const {
  if (laid != Front::Snow || cover <= 0)
    return false;
  float t = 0.25f * float(hash2(x * 3 + 11, y * 5 + 7) % 1000) / 1000 +
            0.75f * (1 - lowland(x, y));
  return t < cover * 1.25f;
}

void Sky::renderLight(Screen &s) const {
  if (laid != Front::Rain || cover < 0.1f)
    return;
  int x0 = int(std::floor((cx - 96) / 16)), y0 = int(std::floor((cy - 72) / 16));
  for (int y = y0; y <= y0 + 9; y++)
    for (int x = x0; x <= x0 + 12; x++) {
      float a = puddle(x, y);
      if (poolAt(x, y)) {
        float dx = x * 16 + 8 - cx, dy = y * 16 + 8 - cy;
        float near = clampf(1 - std::sqrt(dx * dx + dy * dy) / 120, 0, 1);
        float shine = near * 0.35f + flash * 0.9f;
        if (shine > 0.04f)
          s.glow(x * 16 + 8, y * 16 + 8, 20, Color{0.4f, 0.5f, 0.72f, 1} * shine);
        continue;
      }
      if (a < 0.3f)
        continue;
      uint32_t h = hash2(x * 7 + 1, y * 13 + 5);
      float px = x * 16 + 8 + int(h >> 8 & 7) - 4,
            py = y * 16 + 8 + int(h >> 12 & 7) - 4;
      float dx = px - cx, dy = py - cy;
      float near = clampf(1 - std::sqrt(dx * dx + dy * dy) / 110, 0, 1);
      float shine = a * (near * 0.5f + flash * 0.8f);
      if (shine > 0.04f)
        s.glow(px, py - 1, 14, Color{0.45f, 0.55f, 0.75f, 1} * shine);
    }
}
