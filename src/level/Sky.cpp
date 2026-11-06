#include "level/Sky.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "level/Noise.h"
#include "level/World.hpp"

namespace {

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

} // namespace

void Sky::force(Front f) {
  front = next = f, amount = 1, timer = 90, pinned = true;
  laid = groundLayer(f), cover = laid == Front::Clear ? 0.0f : 0.5f;
}

int Sky::active() const { return int(def().drops * amount * surge); }

float Sky::local(float x, float y) const {
  float n = noise_fbm((x - ox) * 0.0032f, (y - oy) * 0.0032f, 2, 909);
  float cells = clampf(0.45f + n * 2.2f, 0, 1);
  cells = cells * cells * (3 - 2 * cells);
  return lerpf(1, cells, def().patchy);
}

float Sky::lowland(int x, int y) {
  struct Slot {
    int x, y;
    uint32_t salt;
    float v;
  };
  static Slot cache[4096];
  Slot &c = cache[(hash2(x, y) ^ noise_salt) & 4095];
  if (c.x != x || c.y != y || c.salt != noise_salt + 1)
    c = {x, y, noise_salt + 1,
         clampf(0.5f + noise_fbm(x * 0.07f + 13.1f, y * 0.07f - 4.7f, 2, 917),
                0, 1)};
  return c.v;
}

float Sky::puddle(int x, int y) const {
  if (laid != Front::Rain || cover <= 0.02f || hash2(x * 7 + 1, y * 13 + 5) % 2 ||
      poolAt(x, y) || world::isWater(x, y))
    return 0;
  return clampf((lowland(x, y) - 0.66f + 0.32f * cover) * 7, 0, 1) * cover;
}

float Sky::flood() const {
  return laid == Front::Rain ? clampf((cover - 0.4f) / 0.6f, 0, 1) : 0;
}

bool Sky::poolAt(int x, int y) const {
  float depth = flood();
  return depth > 0 && lowland(x, y) > 0.92f - 0.2f * depth;
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
  const FrontDef &f = def();
  d.x = cx + rng.range(-w / 2, w / 2);
  d.y = cy + rng.range(-h / 2, h / 2);
  d.depth = rng.range(f.fall == Fall::Rain ? 0.5f : 0.7f, 1);
  d.speed = f.speed * rng.range(0.9f, 1.1f) * (f.fall == Fall::Streak ? 1 : d.depth);
  d.phase = rng.unit(), d.gust = gustAt(d.x, d.y);
  d.age = rng.range(0, 6), d.landed = false;
  d.idle = rng.unit() > local(d.x, d.y);
  float top = f.fall == Fall::Rain ? 240.0f : f.fall == Fall::Flake ? 190.0f : 1.0f;
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

void Sky::seed() {
  seeded = true;
  for (size_t i = 0; i < clouds.size(); i++)
    place(clouds[i], i % 3 ? &clouds[i - i % 3] : nullptr);
  for (size_t i = 0; i < banks.size(); i++)
    place(banks[i], i % 2 ? &banks[i - 1] : nullptr);
}

void Sky::shiftFront(int biome) {
  if ((timer -= DT) <= 0 ||
      (!pinned && next == front && !frontChance(biome, front))) {
    timer = rng.range(45, 100), pinned = false;
    next = pickFront(biome, front);
    if (!frontChance(biome, next))
      next = Front::Clear;
  }
  if (next != front) {
    if ((amount -= DT / 4) <= 0)
      amount = 0, front = next,
      heading = (rng.chance(0.5f) ? 0 : PI) + rng.range(-0.5f, 0.5f);
  } else
    amount = std::min(1.0f, amount + DT / 6);
}

void Sky::blow(float t) {
  const FrontDef &f = def();
  float gust = 0.7f + 0.3f * std::sin(t * 0.07f) + 0.15f * std::sin(t * 0.61f);
  float angle =
      heading + 0.35f * std::sin(t * 0.021f) + 0.15f * std::sin(t * 0.093f);
  windX = lerpf(windX, f.wind * gust * std::cos(angle), DT * 0.5f);
  windY = lerpf(windY, f.wind * gust * std::sin(angle) * 0.6f, DT * 0.5f);
  ox += windX * 0.6f * DT, oy += windY * 0.6f * DT;
  surge = 0.75f + 0.25f * std::sin(t * 0.083f + heading * 3);
}

void Sky::fall(int n) {
  const FrontDef &f = def();
  for (int i = 0; i < n; i++) {
    Drop &d = drops[i];
    d.age += DT;
    if (d.idle) {
      if ((d.z -= DT) <= 0)
        respawn(d, false);
      continue;
    }
    d.gust = lerpf(d.gust, gustAt(d.x, d.y), DT * 3);
    float push = d.gust * (f.fall == Fall::Rain ? 1.2f : f.fall == Fall::Flake ? 0.6f : 5);
    if (f.fall == Fall::Streak) {
      d.x += windX * push * DT, d.y += windY * push * DT;
      if ((d.z -= DT) <= 0)
        respawn(d, false);
    } else if (d.landed) {
      if (d.age > (f.fall == Fall::Rain ? 0.36f : 0.8f))
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
}

void Sky::drift() {
  for (Bank &b : clouds) {
    b.x = wrap(b.x + (windX * 0.8f + 4) * b.speed * DT, cx, w + 440);
    b.y = wrap(b.y + windY * 0.8f * b.speed * DT, cy, h + 320);
  }
  for (Bank &b : banks) {
    b.x = wrap(b.x + windX * 0.4f * b.speed * DT, cx, w + 360);
    b.y = wrap(b.y + windY * 0.4f * b.speed * DT, cy, h + 260);
  }
}

void Sky::settle(int biome) {
  Front want = groundLayer(front);
  if (want != Front::Clear && (laid == want || cover <= 0))
    laid = want, cover = std::min(amount, cover + DT * amount / (front == Front::Storm ? 28 : 50));
  else
    cover = std::max(0.0f, cover - DT / (frontChance(biome, laid) ? 70 : 6));
}

void Sky::tick(Level &level) {
  Player &p = *level.player;
  cx = float(p.x), cy = float(p.y);
  int biome = world::biomeUnder(p.x, p.y);
  if (!seeded)
    seed();
  int before = active();
  shiftFront(biome);
  blow(level.time);
  int n = std::min(active(), int(drops.size()));
  if (level.game.settings.reduced)
    n = n * 2 / 3;
  for (int i = shown == front ? before : 0; i < n; i++)
    respawn(drops[i], true);
  shown = front;
  fall(n);
  drift();
  settle(biome);
  prints.tick(*this, p.x, p.y);
  thunder.tick();
  if (front == Front::Storm && amount >= 0.6f && !p.dead())
    thunder.storm(level);
}

Color Sky::shade() const {
  Color c = mix({1, 1, 1, 1}, def().shade, amount);
  float flash = thunder.flash;
  return {c.r + flash * 0.6f, c.g + flash * 0.65f, c.b + flash * 0.8f, 1};
}

const char *Sky::name() const {
  if (amount < 0.3f)
    return "";
  return front == Front::Rain && amount < 0.7f ? "drizzle" : def().name;
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
