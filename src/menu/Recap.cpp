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

float rankScore(const Level &l) {
  float t = l.time, base;
  if (l.night.dawn)
    base = 10.0f;
  else if (t < 300.0f)
    base = (t / 300.0f) * 2.0f;
  else if (t < 600.0f)
    base = 3.0f + ((t - 300.0f) / 300.0f) * 2.0f;
  else
    base = 6.0f + (std::min(t - 600.0f, 300.0f) / 300.0f) * 2.0f;
  float dmgBonus = std::min(l.totalDamage / 60'000.0f, 4.0f);
  float killBonus = std::min(float(l.kills) / 250.0f, 3.0f);
  float shrineBonus = std::min(float(l.night.lit) * 0.6f, 2.4f);
  float distBonus = std::min(l.distanceRan / 16.0f / 2500.0f, 1.0f);
  return base + dmgBonus + killBonus + shrineBonus + distBonus;
}

const RankInfo &rankOf(const Level &l) {
  float raw = rankScore(l);
  float handicapBonus = std::min(float(l.handicaps) * 0.8f, 2.5f);
  int rawIndex = std::clamp(int(std::floor(raw)), 0, 20);
  int boostedIndex = std::clamp(int(std::floor(raw + handicapBonus)), 0, 17);
  return RANKS[std::max(rawIndex, boostedIndex)];
}

std::string formatDistance(float metres) {
  if (metres < 1000.0f)
    return fmt::format("{} m", int(std::round(metres)));
  return fmt::format("{:.2f} km", metres / 1000.0f);
}

std::string formatDamage(float damage) {
  if (damage >= 1'000'000.0f)
    return fmt::format("{:.2f}M", damage / 1'000'000.0f);
  if (damage >= 10'000.0f)
    return fmt::format("{:.1f}k", damage / 1'000.0f);
  return fmt::format("{}", int(std::round(damage)));
}

void corners(Screen &s, float x, float y, float w, float h, float tick, float u,
             Color c) {
  for (int i = 0; i < 4; i++) {
    float cx = i & 1 ? x + w : x, cy = i & 2 ? y + h : y;
    float hx = i & 1 ? cx - tick : cx + tick, vy = i & 2 ? cy - tick : cy + tick;
    s.line(cx, cy, hx, cy, u, c);
    s.line(cx, cy, cx, vy, u, c);
  }
}

struct Tip {
  bool (*fits)(const Level &l);
  const char *text;
};

int weapons(const Level &l, bool maxed) {
  int n = 0;
  for (auto &up : l.player->upgrades)
    n += up->weapon && up->level > 0 && (!maxed || up->maxed());
  return n;
}

const Tip TIPS[] = {
    {[](const Level &l) { return l.time < 60; },
     "Keep moving from the first second: foes close in on a hero who stands still"},
    {[](const Level &l) { return !l.bossIds.empty(); },
     "Bosses telegraph their heaviest blows: step out of the warning ring before it fills"},
    {[](const Level &l) { return l.rules.endsAtDawn && l.night.lit == 0 && l.time > 180; },
     "Kindle shrines: every shrine lit brings the dawn sooner"},
    {[](const Level &l) { return weapons(l, false) >= 3 && weapons(l, true) == 0; },
     "Raise one weapon to level 5 before spreading out: maxed weapons can evolve"},
    {[](const Level &l) { return l.distanceRan < l.time * 25; },
     "Keep moving: walking through the gaps keeps the crowd from closing the circle"},
    {[](const Level &l) { return l.player->lvl * 30 < l.time; },
     "Gather the gems: a wider magnet and more experience level you faster"},
    {[](const Level &) { return true; },
     "Pair a maxed weapon with its partner relic to evolve it"}};

} // namespace

RunStats computeRunStats(const Level &l) {
  RunStats stats;
  const RankInfo &rank = rankOf(l);
  stats.rank = rank.grade;
  stats.rankColor = rank.color;
  stats.distance = formatDistance(l.distanceRan / 16.0f);
  stats.totalDamage = formatDamage(l.totalDamage);

  const Hero *h = l.player ? l.player->hero : nullptr;
  stats.className = h ? h->name : "Adventurer";
  stats.classColor = h ? h->tint : col::white;

  stats.difficulty = difficultyTitle(l.game.difficulty);
  stats.diffColor = {0.82f, 0.85f, 0.92f, 1.0f};
  if (l.omen >= 0 && l.omen < OMEN_COUNT)
    stats.omen = OMENS[l.omen].name, stats.omenColor = OMENS[l.omen].tint;

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

  corners(s, bx, by, badgeW, badgeH, 6 * u, u, col::white.alpha(alpha));

  s.text("RANK", bx + 16 * u, by + (badgeH - 16 * small) * 0.5f, small, col::muted.alpha(alpha));
  s.textCenter(stats.rank, bx + badgeW - 38 * u, by + (badgeH - 16 * 1.8f * u) * 0.5f, 1.8f * u, stats.rankColor.alpha(alpha));

  float cardW = std::min(s.w * 0.88f, 340 * u);
  float cardH = 46 * u;
  float cx = (s.w - cardW) * 0.5f;
  float cy = by + badgeH + 8 * u;

  gui::Panel({cx, cy, cardW, cardH}, col::panel.alpha(0.90f * alpha),
             col::muted.alpha(0.35f * alpha), u).render(s);

  auto field = [&](const char *label, const std::string &value, float x, float y,
                   float gap, Color c) {
    s.text(label, x, y, small, col::muted.alpha(0.75f * alpha));
    s.text(value, x + gap * u, y, small, c.alpha(alpha));
  };
  float top = cy + 4.5f * u, bottom = cy + 26.5f * u;
  field("CLASS", stats.className, cx + 10 * u, top, 44, stats.classColor);
  field("DIFF", stats.difficulty, cx + cardW * 0.35f, top, 36, stats.diffColor);
  field("RAN", stats.distance, cx + cardW * 0.70f, top, 30, col::white);
  s.line(cx + 8 * u, cy + 23 * u, cx + cardW - 8 * u, cy + 23 * u, 1,
         col::muted.alpha(0.2f * alpha));
  field("DMG", stats.totalDamage, cx + 10 * u, bottom, 32, col::white);
  field("FOES", std::to_string(stats.kills), cx + cardW * 0.28f, bottom, 38, col::white);
  field("LIT", std::to_string(stats.shrines), cx + cardW * 0.54f, bottom, 28, col::gold);
  field("TIME", stats.timeText, cx + cardW * 0.73f, bottom, 34,
        stats.isRecord ? col::gold : col::white);

  if (stats.omen.empty())
    return cy + cardH;
  s.textCenter("under the omen " + stats.omen, s.w / 2, cy + cardH + 5 * u, small,
               stats.omenColor.alpha(0.85f * alpha));
  return cy + cardH + 5 * u + 16 * small;
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

const char *advice(const Level &l) {
  for (const Tip &tip : TIPS)
    if (tip.fits(l))
      return tip.text;
  return "";
}
