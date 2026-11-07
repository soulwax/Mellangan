#include "level/Matter.hpp"

#include <cmath>
#include <vector>

#include "gfx/Screen.hpp"
#include "level/Level.hpp"

namespace {

int floorDiv(int v, int d) { return (v - (v < 0 ? d - 1 : 0)) / d; }

struct Expiry {
  Stuff next;
  float seconds;
};

constexpr Expiry EXPIRE[int(Stuff::Count)] = {
    {Stuff::None, 0}, {Stuff::None, 0}, {Stuff::Ash, 20}, {Stuff::Wet, 6}, {Stuff::None, 0},
    {Stuff::None, 0}, {Stuff::Wet, 4},  {Stuff::None, 0}, {Stuff::None, 0},
};

constexpr Color LOOK[int(Stuff::Count)] = {
    {0, 0, 0, 0},           {0.28f, 0.48f, 0.86f, 0.26f}, {1, 0.48f, 0.14f, 0.34f},
    {0.8f, 0.92f, 1, 0.44f}, {0.16f, 0.12f, 0.1f, 0.4f},    {0.62f, 0.4f, 0.78f, 0.3f},
    {0.6f, 0.8f, 1, 0.3f},  {0.16f, 0.15f, 0.15f, 0.46f}, {0.92f, 0.92f, 0.86f, 0.4f},
};

} // namespace

const Matter::Page *Matter::find(int x, int y) const {
  auto it = pages.find(keyOf(floorDiv(x, PAGE), floorDiv(y, PAGE)));
  return it == pages.end() ? nullptr : it->second.get();
}

Stuff Matter::at(int x, int y) const {
  const Page *p = find(x, y);
  return p ? Stuff(p->kind[(y - floorDiv(y, PAGE) * PAGE) * PAGE + x - floorDiv(x, PAGE) * PAGE]) : Stuff::None;
}

float Matter::left(int x, int y) const {
  const Page *p = find(x, y);
  return p ? p->time[(y - floorDiv(y, PAGE) * PAGE) * PAGE + x - floorDiv(x, PAGE) * PAGE] * STEP : 0;
}

void Matter::set(int x, int y, Stuff s, float seconds) {
  int px = floorDiv(x, PAGE), py = floorDiv(y, PAGE), i = (y - py * PAGE) * PAGE + x - px * PAGE;
  auto it = pages.find(keyOf(px, py));
  if (it == pages.end()) {
    if (s == Stuff::None)
      return;
    it = pages.emplace(keyOf(px, py), std::make_unique<Page>()).first;
  }
  Page &p = *it->second;
  p.live += (s != Stuff::None) - (p.kind[i] != uint8_t(Stuff::None));
  p.kind[i] = uint8_t(s), p.time[i] = uint16_t(std::clamp(seconds / STEP, 0.0f, 60000.0f));
  if (!p.live)
    pages.erase(it);
}

void Matter::tick(Level &level) {
  if (pages.empty() || (clock += DT) < STEP)
    return;
  clock -= STEP;
  bool spread = ++steps % SPREAD == 0;
  struct Change {
    int x, y;
    Stuff s;
    float seconds;
  };
  std::vector<Change> changes;
  for (auto &[key, page] : pages) {
    int px = int(int32_t(key >> 32)), py = int(int32_t(key & 0xFFFFFFFFu));
    for (int i = 0; i < PAGE * PAGE; i++) {
      Stuff s = Stuff(page->kind[i]);
      if (s == Stuff::None)
        continue;
      int x = px * PAGE + i % PAGE, y = py * PAGE + i / PAGE;
      if (page->time[i] > 0 && --page->time[i] == 0) {
        const Expiry &e = EXPIRE[int(s)];
        changes.push_back({x, y, e.next, e.seconds});
        if (s == Stuff::Burning)
          level.scorch(x, y);
        continue;
      }
      if (spread && s == Stuff::Burning && fuel) {
        constexpr int D[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (const auto &d : D)
          if (at(x + d[0], y + d[1]) == Stuff::None && fuel(x + d[0], y + d[1]))
            changes.push_back({x + d[0], y + d[1], Stuff::Burning, Level::BURN});
      }
    }
  }
  for (const Change &c : changes)
    set(c.x, c.y, c.s, c.seconds);
}

void Matter::render(Screen &s, float time) const {
  if (pages.empty())
    return;
  int x0 = floorDiv(int(std::floor(s.xOffset / 16)), PAGE), y0 = floorDiv(int(std::floor(s.yOffset / 16)), PAGE);
  int x1 = floorDiv(int(std::floor((s.xOffset + s.vw) / 16)), PAGE), y1 = floorDiv(int(std::floor((s.yOffset + s.vh) / 16)), PAGE);
  for (int py = y0; py <= y1; py++)
    for (int px = x0; px <= x1; px++) {
      auto it = pages.find(keyOf(px, py));
      if (it == pages.end())
        continue;
      const Page &p = *it->second;
      for (int i = 0; i < PAGE * PAGE; i++) {
        Stuff k = Stuff(p.kind[i]);
        if (k == Stuff::None)
          continue;
        int x = px * PAGE + i % PAGE, y = py * PAGE + i / PAGE;
        Color c = LOOK[int(k)];
        if (k == Stuff::Burning)
          c.a *= 0.75f + 0.25f * std::sin(time * 11 + x * 1.7f + y * 2.3f);
        s.rect(x * 16.0, y * 16.0, 16, 16, c);
      }
    }
}
