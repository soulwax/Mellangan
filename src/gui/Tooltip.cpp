#include "gui/Tooltip.hpp"

#include "gfx/Palette.hpp"
#include "item/loot/Unique.hpp"

#include <spdlog/fmt/fmt.h>

#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "item/craft/Recipes.hpp"
#include "item/loot/Item.hpp"
#include "item/loot/Vendor.hpp"

namespace gui {

namespace {

constexpr Color AFFIX{0.55f, 0.65f, 1, 1}, CURSE{1, 0.3f, 0.3f, 1};

typedef std::vector<std::pair<std::string, Color>> Lines;

void compare(Lines &lines, const Item &item, const Item &worn) {
  bool header = false;
  for (int s = int(Stat::Damage); s <= int(Stat::Light); s++) {
    float delta = item.stat(Stat(s)) - worn.stat(Stat(s));
    if (std::abs(delta) < 0.05f)
      continue;
    if (!header)
      lines.push_back({"against what you wear:", col::muted}), header = true;
    lines.push_back({(delta > 0 ? "" : "lose ") + describe(Stat(s), std::abs(delta)),
                     delta > 0 ? palette::better() : palette::worse()});
  }
}

} // namespace

void itemTooltip(Screen &s, const Item &item, const Item *worn, const char *hero, float x,
                 float y, int cost) {
  const ItemBase &b = item.def();
  Lines lines{{item.name(), item.color()}};
  if (item.rarity == Rarity::Rare || item.rarity == Rarity::Crafted)
    lines.push_back({item.baseName(), col::white});
  lines.push_back({fmt::format("{}  item level {}", gearName(item.gearSlot()), item.ilvl),
                   col::muted});
  if (const UniqueDef *u = uniqueOf(item))
    lines.push_back({u->text, item.color()});
  else if (b.grants)
    lines.push_back({fmt::format("grants {}", b.grants), col::white});
  if (b.implicit != Stat::None)
    lines.push_back({describe(b.implicit, item.implicit()), col::white});
  for (int i = 0; i < item.count; i++)
    lines.push_back(
        {describe(AFFIXES[item.rolls[i].affix].stat, item.rolls[i].value), AFFIX});
  if (item.night())
    lines.push_back({fmt::format("{}: {}", CURSES[item.curse].name, CURSES[item.curse].text),
                     CURSE});
  if (b.klass) {
    bool fits = hero && std::strcmp(hero, b.klass) == 0;
    lines.push_back({fmt::format("{} only", b.klass), fits ? col::muted : CURSE});
  }
  if (cost >= 0)
    lines.push_back({fmt::format("costs {} gold", cost), col::gold});
  else
    lines.push_back({fmt::format("sells for {} gold", Vendor::sellPrice(item)), col::muted});
  if (worn)
    compare(lines, item, *worn);

  float u = s.u, small = std::max(1.0f, u - 1), line = 16 * small + 2 * u, pad = 5 * u;
  float w = 0;
  for (auto &[text, c] : lines)
    w = std::max(w, Screen::textWidth(text, small));
  w += pad * 2;
  float h = lines.size() * line + pad * 2;
  x = std::clamp(x, 2 * u, s.w - w - 2 * u);
  y = std::clamp(y, 2 * u, s.h - h - 2 * u);
  s.rect(x, y, w, h, {0.03f, 0.03f, 0.06f, 0.95f});
  s.frame(x, y, w, h, u, item.color().alpha(0.7f));
  for (size_t i = 0; i < lines.size(); i++)
    s.text(lines[i].first, x + pad, y + pad + i * line, small, lines[i].second);
}

float materials(Screen &s, const int *owned, const int *cost, float x, float y, float size,
               float alpha) {
  for (int m = 0; m < MATS; m++) {
    if (cost && !cost[m])
      continue;
    std::string n = cost ? fmt::format("{}/{}", owned[m], cost[m]) : std::to_string(owned[m]);
    Color ink = cost && owned[m] < cost[m] ? palette::worse() : MATERIALS[m].color;
    s.sprite(MATERIALS[m].icon, x, y - 6 * size, 0, 0, size, size);
    s.text(n, x + 17 * size, y, size, ink.alpha(alpha));
    x += 17 * size + Screen::textWidth(n, size) + 9 * size;
  }
  return x;
}

} // namespace gui
