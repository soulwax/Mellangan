#include "menu/Recap.hpp"

#include <algorithm>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

float recap(Screen &s, const Level &l, float y, float alpha) {
  const Player &p = *l.player;
  Upgrade *top[8];
  int n = 0;
  float total = 0;
  for (auto &up : p.upgrades)
    if (up->dealt > 0) {
      total += up->dealt;
      if (n < 8)
        top[n++] = up.get();
    }
  if (n == 0)
    return y;
  std::sort(top, top + n,
            [](const Upgrade *a, const Upgrade *b) { return a->dealt > b->dealt; });
  n = std::min(n, 5);

  float u = s.u, small = std::max(1.0f, u - 1), row = 20 * u;
  float w = std::min(s.w * 0.8f, 300 * u), x = (s.w - w) / 2, bar = w - 170 * u;
  s.textCenter("DAMAGE DEALT", s.w / 2, y, small, col::muted.alpha(alpha));
  y += 18 * u;
  for (int i = 0; i < n; i++) {
    const Upgrade *up = top[i];
    float share = up->dealt / total;
    Color c = up->rare ? col::gold : col::white;
    s.color(col::white.alpha(alpha));
    s.sprite(up->icon, x, y, 0, 0, u, u);
    s.color(col::white);
    s.text(up->name, x + 20 * u, y + 2 * u, small, c.alpha(alpha));
    float bx = x + 130 * u;
    s.rect(bx, y + 6 * u, bar, 5 * u, col::panel.alpha(alpha));
    s.rect(bx, y + 6 * u, bar * share, 5 * u,
           (up->rare ? col::gold : col::xp).alpha(alpha));
    std::string pct = fmt::format("{}%", int(std::lround(share * 100)));
    s.text(pct, x + w - Screen::textWidth(pct, small), y + 2 * u, small,
           col::muted.alpha(alpha));
    y += row;
  }
  return y;
}
