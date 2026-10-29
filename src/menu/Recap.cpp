#include "menu/Recap.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cmath>

#include "Game.hpp"
#include "gui/Widget.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "level/Flavour.hpp"
#include "level/Level.hpp"

namespace {
struct RankInfo {
  const char *grade;
  Color color;
};

const RankInfo RANKS[21] = {
    {"F-", {0.80f, 0.22f, 0.22f, 1.0f}},
    {"F",  {0.85f, 0.25f, 0.25f, 1.0f}},
    {"F+", {0.90f, 0.30f, 0.25f, 1.0f}},
    {"E-", {0.85f, 0.42f, 0.22f, 1.0f}},
    {"E",  {0.88f, 0.48f, 0.24f, 1.0f}},
    {"E+", {0.92f, 0.55f, 0.26f, 1.0f}},
    {"D-", {0.85f, 0.65f, 0.28f, 1.0f}},
    {"D",  {0.88f, 0.72f, 0.30f, 1.0f}},
    {"D+", {0.92f, 0.78f, 0.32f, 1.0f}},
    {"C-", {0.70f, 0.75f, 0.82f, 1.0f}},
    {"C",  {0.78f, 0.82f, 0.88f, 1.0f}},
    {"C+", {0.85f, 0.88f, 0.94f, 1.0f}},
    {"B-", {0.28f, 0.75f, 0.45f, 1.0f}},
    {"B",  {0.30f, 0.85f, 0.50f, 1.0f}},
    {"B+", {0.35f, 0.92f, 0.55f, 1.0f}},
    {"A-", {0.30f, 0.65f, 0.95f, 1.0f}},
    {"A",  {0.32f, 0.75f, 1.0f,  1.0f}},
    {"A+", {0.40f, 0.85f, 1.0f,  1.0f}},
    {"S-", {0.95f, 0.75f, 0.28f, 1.0f}},
    {"S",  {1.0f,  0.84f, 0.22f, 1.0f}},
    {"S+", {1.0f,  0.92f, 0.35f, 1.0f}}
};
}

RunStats computeRunStats(const Level &l) {
  RunStats stats;

  float t = l.time;
  float base = 0.0f;
  if (l.night.dawn) {
    base = 10.0f;
  } else if (t < 300.0f) {
    base = (t / 300.0f) * 2.0f;
  } else if (t < 600.0f) {
    base = 3.0f + ((t - 300.0f) / 300.0f) * 2.0f;
  } else {
    base = 6.0f + (std::min(t - 600.0f, 300.0f) / 300.0f) * 2.0f;
  }

  float dmgBonus = std::min(l.totalDamage / 60'000.0f, 4.0f);
  float killBonus = std::min(float(l.kills) / 250.0f, 3.0f);
  float shrineBonus = std::min(float(l.night.lit) * 0.6f, 2.4f);
  float distMetres = l.distanceRan / 16.0f;
  float distBonus = std::min(distMetres / 2500.0f, 1.0f);

  float rawScore = base + dmgBonus + killBonus + shrineBonus + distBonus;

  float handicapBonus = std::min(float(l.handicaps) * 0.8f, 2.5f);

  int rawIndex = std::clamp(int(std::floor(rawScore)), 0, 20);
  int boostedIndex = std::clamp(int(std::floor(rawScore + handicapBonus)), 0, 17);
  int finalIndex = std::max(rawIndex, boostedIndex);

  stats.rank = RANKS[finalIndex].grade;
  stats.rankColor = RANKS[finalIndex].color;

  if (distMetres < 1000.0f)
    stats.distance = fmt::format("{} m", int(std::round(distMetres)));
  else
    stats.distance = fmt::format("{:.2f} km", distMetres / 1000.0f);

  if (l.totalDamage >= 1'000'000.0f)
    stats.totalDamage = fmt::format("{:.2f}M", l.totalDamage / 1'000'000.0f);
  else if (l.totalDamage >= 10'000.0f)
    stats.totalDamage = fmt::format("{:.1f}k", l.totalDamage / 1'000.0f);
  else
    stats.totalDamage = fmt::format("{}", int(std::round(l.totalDamage)));

  const Hero *h = l.player ? l.player->hero : nullptr;
  stats.className = h ? h->name : "Adventurer";
  stats.classColor = h ? h->tint : col::white;

  if (l.omen >= 0 && l.omen < OMEN_COUNT) {
    stats.difficulty = OMENS[l.omen].name;
    stats.diffColor = OMENS[l.omen].tint;
  } else {
    stats.difficulty = "STANDARD";
    stats.diffColor = {0.82f, 0.85f, 0.92f, 1.0f};
  }

  stats.timeText = clockText(l.time);
  stats.isRecord = l.time > l.game.best;
  stats.kills = l.kills;
  stats.shrines = l.night.lit;
  stats.level = l.player ? l.player->lvl : 1;

  return stats;
}

float renderEndStats(Screen &s, const Level &l, float y, float alpha) {
  RunStats stats = computeRunStats(l);
  float u = s.u, small = std::max(1.0f, u - 1);

  float badgeW = 160 * u, badgeH = 34 * u;
  float bx = (s.w - badgeW) * 0.5f, by = y;

  s.rect(bx, by, badgeW, badgeH, col::panel.alpha(0.92f * alpha));
  s.frame(bx, by, badgeW, badgeH, 2 * u, stats.rankColor.alpha(0.9f * alpha));

  float tick = 6 * u;
  s.line(bx, by, bx + tick, by, u, col::white.alpha(alpha));
  s.line(bx, by, bx, by + tick, u, col::white.alpha(alpha));
  s.line(bx + badgeW, by, bx + badgeW - tick, by, u, col::white.alpha(alpha));
  s.line(bx + badgeW, by, bx + badgeW, by + tick, u, col::white.alpha(alpha));
  s.line(bx, by + badgeH, bx + tick, by + badgeH, u, col::white.alpha(alpha));
  s.line(bx, by + badgeH, bx, by + badgeH - tick, u, col::white.alpha(alpha));
  s.line(bx + badgeW, by + badgeH, bx + badgeW - tick, by + badgeH, u, col::white.alpha(alpha));
  s.line(bx + badgeW, by + badgeH, bx + badgeW, by + badgeH - tick, u, col::white.alpha(alpha));

  s.text("RANK", bx + 16 * u, by + (badgeH - 16 * small) * 0.5f, small, col::muted.alpha(alpha));
  s.textCenter(stats.rank, bx + badgeW - 38 * u, by + (badgeH - 16 * 1.8f * u) * 0.5f, 1.8f * u, stats.rankColor.alpha(alpha));

  float cardW = std::min(s.w * 0.88f, 340 * u);
  float cardH = 46 * u;
  float cx = (s.w - cardW) * 0.5f;
  float cy = by + badgeH + 8 * u;

  gui::Panel({cx, cy, cardW, cardH}, col::panel.alpha(0.90f * alpha),
             col::muted.alpha(0.35f * alpha), u).render(s);

  float row1Y = cy + 4.5f * u;
  float col1X = cx + 10 * u;
  float col2X = cx + cardW * 0.35f;
  float col3X = cx + cardW * 0.70f;

  s.text("CLASS", col1X, row1Y, small, col::muted.alpha(0.75f * alpha));
  s.text(stats.className, col1X + 44 * u, row1Y, small, stats.classColor.alpha(alpha));

  s.text("DIFF", col2X, row1Y, small, col::muted.alpha(0.75f * alpha));
  s.text(stats.difficulty, col2X + 36 * u, row1Y, small, stats.diffColor.alpha(alpha));

  s.text("RAN", col3X, row1Y, small, col::muted.alpha(0.75f * alpha));
  s.text(stats.distance, col3X + 30 * u, row1Y, small, col::white.alpha(alpha));

  s.line(cx + 8 * u, cy + 23 * u, cx + cardW - 8 * u, cy + 23 * u, 1, col::muted.alpha(0.2f * alpha));

  float row2Y = cy + 26.5f * u;
  float c1 = cx + 10 * u;
  float c2 = cx + cardW * 0.28f;
  float c3 = cx + cardW * 0.54f;
  float c4 = cx + cardW * 0.73f;

  s.text("DMG", c1, row2Y, small, col::muted.alpha(0.75f * alpha));
  s.text(stats.totalDamage, c1 + 32 * u, row2Y, small, col::white.alpha(alpha));

  s.text("FOES", c2, row2Y, small, col::muted.alpha(0.75f * alpha));
  s.text(std::to_string(stats.kills), c2 + 38 * u, row2Y, small, col::white.alpha(alpha));

  s.text("LIT", c3, row2Y, small, col::muted.alpha(0.75f * alpha));
  s.text(std::to_string(stats.shrines), c3 + 28 * u, row2Y, small, col::gold.alpha(alpha));

  s.text("TIME", c4, row2Y, small, col::muted.alpha(0.75f * alpha));
  s.text(stats.timeText, c4 + 34 * u, row2Y, small, (stats.isRecord ? col::gold : col::white).alpha(alpha));

  return cy + cardH;
}

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
  n = std::min(n, 4);

  float u = s.u, small = std::max(1.0f, u - 1), row = 18 * u;
  float w = std::min(s.w * 0.88f, 340 * u), x = (s.w - w) / 2, bar = w - 170 * u;
  s.textCenter("WEAPON DAMAGE", s.w / 2, y, small, col::muted.alpha(alpha));
  y += 16 * u;
  for (int i = 0; i < n; i++) {
    const Upgrade *up = top[i];
    float share = total > 0 ? up->dealt / total : 0;
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
