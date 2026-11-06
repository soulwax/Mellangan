#include "menu/UpgradeCard.hpp"

#include <spdlog/fmt/fmt.h>

#include <cstring>
#include <string>

#include "entity/Player.hpp"
#include "gui/Glyph.hpp"
#include "gui/Widget.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"

namespace {

const char *const SYMBOLS[6][11] = {
    {"#.........#", ".#.......#.", "..#.....#..", "...#...#...", "....#.#....", ".....#.....",
     "...#####...", "..#..#..#..", ".#...#...#.", "#....#....#", "..........."},
    {"...........", ".####.####.", "#++++#++++#", "#++++#++++#", "#++++#++++#", "#++++#++++#",
     "#.---+---.#", "..#--+--#..", "...#---#...", "....###....", ".....#....."},
    {"...........", ".#...#...#.", ".#...#...#.", ".##..#..##.", "#++#+#+#++#", "#+++++++++#",
     "#+#+#*#+#+#", "#+++++++++#", ".#########.", "...........", "..........."},
    {".....#.....", "....#+#....", "...#+++#...", "..#++-++#..", ".#+--#--+#.", "....#+#....",
     "...#+++#...", "..#++-++#..", ".#+--#--+#.", "..#--#--#..", "..........."},
    {".....#.....", "....#+#....", "...#+++#...", "..#+++++#..", ".#+++++++#.", "##+++*+++##",
     ".#+++++++#.", "..#+++++#..", "...#+++#...", "....#+#....", ".....#....."},
    {"...........", ".###...###.", "#+++#.#+++#", "#++++#++++#", "#+++++++++#", ".#+++++++#.",
     "..#+++++#..", "...#+++#...", "....#+#....", ".....#.....", "..........."}};

const Color TIERS[5] = {{0.68f, 0.70f, 0.76f, 1.0f},
                        {0.96f, 0.97f, 1.0f, 1.0f},
                        {0.28f, 0.88f, 0.42f, 1.0f},
                        {0.26f, 0.64f, 0.98f, 1.0f},
                        {0.78f, 0.38f, 0.98f, 1.0f}};

const Color SILVER{0.90f, 0.94f, 1.0f, 1};

const gui::GlyphInk INK{0.6f, {0.02f, 0.02f, 0.04f, 0.75f}, 0.4f, 0.6f, 3.0f, 1.0f};

bool same(const char *a, const char *b) { return a && b && std::strcmp(a, b) == 0; }

} // namespace

UpgradeCard::UpgradeCard(const Upgrade *up, const Player &p, bool evolving, int index)
    : up(up), p(p), evolving(evolving), index(index) {
  if (!up) {
    symbol = 5, tint = {0.32f, 0.90f, 0.48f, 1.0f};
    return;
  }
  classy = p.hero && (same(up->owner, p.hero->name) || same(p.hero->weapon, up->name));
  if (up->rare)
    symbol = 2, tint = {1.0f, 0.58f, 0.16f, 1.0f};
  else if (up->skill)
    symbol = 4, tint = {0.45f, 0.92f, 1.0f, 1.0f}, classy = true;
  else
    symbol = evolving ? 3 : classy ? 4 : up->weapon ? 0 : 1,
    tint = TIERS[std::clamp(evolving ? 5 : up->level + 1, 1, 5) - 1];
}

void UpgradeCard::renderThread(Screen &s, SDL_FRect r, float in, float t) const {
  float u = s.u;
  float ix = r.x + 2.0f * u, iy = r.y + 2.0f * u, iw = r.w - 4.0f * u, ih = r.h - 4.0f * u;
  Color base{0.78f, 0.82f, 0.92f, 0.60f * in};
  Color gleam{0.96f, 0.98f, 1.0f, in};

  s.line(ix + 6 * u, iy, ix + iw - 6 * u, iy, u, base);
  s.line(ix + 6 * u, iy + ih, ix + iw - 6 * u, iy + ih, u, base);
  s.line(ix, iy + 4 * u, ix, iy + ih - 4 * u, u, base);
  s.line(ix + iw, iy + 4 * u, ix + iw, iy + ih - 4 * u, u, base);

  s.line(ix, iy + 4 * u, ix + 6 * u, iy, u, base);
  s.rect(ix + 2 * u, iy + 2 * u, u, u, gleam);
  s.line(ix + iw - 6 * u, iy, ix + iw, iy + 4 * u, u, base);
  s.rect(ix + iw - 3 * u, iy + 2 * u, u, u, gleam);
  s.line(ix, iy + ih - 4 * u, ix + 6 * u, iy + ih, u, base);
  s.rect(ix + 2 * u, iy + ih - 3 * u, u, u, gleam);
  s.line(ix + iw - 6 * u, iy + ih, ix + iw, iy + ih - 4 * u, u, base);
  s.rect(ix + iw - 3 * u, iy + ih - 3 * u, u, u, gleam);

  float pulse = fract(t * 0.42f + index * 0.22f);
  float glowLen = 6 * u;
  float gxTop = ix + 8 * u + pulse * (iw - 16 * u);
  s.line(gxTop - glowLen, iy, gxTop + glowLen, iy, u,
         gleam.alpha((0.65f + 0.35f * std::sin(t * 6.0f)) * in));
  s.rect(gxTop - 0.5f * u, iy - 0.5f * u, 2 * u, 2 * u, col::white.alpha(in));
  float gxBot = ix + iw - 8 * u - pulse * (iw - 16 * u);
  s.line(gxBot - glowLen, iy + ih, gxBot + glowLen, iy + ih, u,
         gleam.alpha((0.65f + 0.35f * std::sin(t * 6.0f + 3.14f)) * in));
  s.rect(gxBot - 0.5f * u, iy + ih - 0.5f * u, 2 * u, 2 * u, col::white.alpha(in));
}

void UpgradeCard::renderTag(Screen &s, float right, float y, float in) const {
  float u = s.u, small = std::max(1.0f, u - 1);
  auto tag = [&](const std::string &text, Color c) {
    s.text(text, right - Screen::textWidth(text, small), y, small, c);
  };
  const char *hero = p.hero ? p.hero->name : "CLASS";
  if (evolving)
    return tag("EVOLVE", col::xp.alpha(in));
  if (!up)
    return;
  if (up->skill)
    return tag(fmt::format("SKILL  REPLACES {}", p.skill ? p.skill->name : "NONE"),
               tint.alpha(in));
  if (up->rare)
    return tag("RELIC", tint.alpha(in));
  if (up->level == 0)
    return classy ? tag(fmt::format("{}  TIER 1", hero), SILVER.alpha(in))
                  : tag("TIER 1 (NEW)", tint.alpha(in));
  std::string tier = fmt::format("T{}", up->level + 1);
  float tierW = Screen::textWidth(tier, small);
  tag(tier, tint.alpha(in));
  float pipW = Upgrade::MAX_LEVEL * 4 * u, pipsX = right - tierW - pipW - 4 * u;
  for (int lvl = 0; lvl < Upgrade::MAX_LEVEL; lvl++)
    s.rect(pipsX + lvl * 4 * u, y + 6.0f * u, 2.5f * u, 3.5f * u,
           (lvl <= up->level ? tint : Color{0.22f, 0.20f, 0.30f, 1}).alpha(in));
  if (classy)
    s.text(hero, pipsX - Screen::textWidth(hero, small) - 5 * u, y, small,
           SILVER.alpha(in));
}

std::string UpgradeCard::synergy(bool &ready) const {
  ready = false;
  if (!up || evolving || up->evolved)
    return "";
  auto held = [&](const char *name) {
    Upgrade *u = p.upgrade(name);
    return u && u->level > 0;
  };
  for (const auto &e : evolutionTable()) {
    if (up->weapon && std::strcmp(e[0], up->name) == 0)
      return ready = held(e[1]), fmt::format("evolves with {}{}", e[1], ready ? ", which you hold" : "");
    if (!up->weapon && std::strcmp(e[1], up->name) == 0 && held(e[0]))
      return ready = true, fmt::format("evolves your {}", e[0]);
  }
  return "";
}

std::vector<std::string> UpgradeCard::numbers() const {
  std::vector<std::string> out;
  if (!up)
    return out;
  bool fresh = up->level == 0 && !evolving;
  int next = evolving ? up->level : std::min(up->level + 1, Upgrade::MAX_LEVEL);
  float power = evolving ? Upgrade::EVOLVED_POWER : up->power,
        pace = evolving ? Upgrade::EVOLVED_PACE : up->pace;
  Figures now = up->figures(up->level, up->evolved), then = up->figures(next, evolving || up->evolved);
  auto show = [&](const Figure &st, float pw, float pc) {
    auto seconds = [](float v) { return v < 1 ? fmt::format("{:.2f}s", v) : fmt::format("{:.1f}s", v); };
    switch (st.kind) {
    case Figure::Damage:
      return fmt::format("{:.0f}", st.value * p.might * pw);
    case Figure::Every:
      return seconds(st.value * p.haste * pc);
    case Figure::Lasts:
      return seconds(st.value);
    case Figure::Percent:
      return fmt::format("{:.0f}%", st.value);
    default:
      return fmt::format("{:.0f}", st.value);
    }
  };
  for (int i = 0; i < then.n; i++) {
    std::string b = show(then.line[i], power, pace);
    if (fresh) {
      out.push_back(fmt::format("{} {}", then.line[i].name, b));
      continue;
    }
    std::string a = show(now.line[i], up->power, up->pace);
    if (a != b)
      out.push_back(fmt::format("{} {} > {}", then.line[i].name, a, b));
  }
  return out;
}

void UpgradeCard::render(Screen &s, SDL_FRect r, float in, bool selected, float t) const {
  float u = s.u, small = std::max(1.0f, u - 1);
  float x = r.x, y = r.y, w = r.w, h = r.h;
  gui::ChoiceCard(r, tint, u, in, selected).render(s);
  if (classy)
    renderThread(s, r, in, t);

  float badge = 24 * u;
  float bx = x + 6 * u, by = y + (h - badge) * 0.5f;
  s.rect(bx, by, badge, badge, {tint.r * 0.16f, tint.g * 0.16f, tint.b * 0.16f, 0.9f * in});
  s.frame(bx, by, badge, badge, u, tint.alpha(0.8f * in));
  if (classy)
    s.frame(bx - u, by - u, badge + 2 * u, badge + 2 * u, u,
            Color{0.90f, 0.95f, 1.0f, (0.45f + 0.35f * std::sin(t * 4.0f)) * in});
  gui::glyph(s, SYMBOLS[symbol], 11, bx + (badge - 11 * u) * 0.5f,
             by + (badge - 11 * u) * 0.5f, u, tint.alpha(in), t, INK);

  float iconX = bx + badge + 6 * u, iconY = y + (h - 16 * u) * 0.5f;
  s.color(col::white.alpha(in));
  s.sprite(up ? up->icon : atlas::tile(atlas::t::PotionRed), iconX, iconY, 0, 0, u, u);
  s.color(col::white);

  float tx = iconX + 16 * u + 7 * u;
  const char *name = evolving ? up->evolvedName : up ? up->name : "Roast Feast";
  std::string desc = evolving ? fmt::format("{} evolves: double damage, faster", up->name)
                     : up     ? up->desc[std::min(up->level, Upgrade::MAX_LEVEL - 1)]
                              : "Restore 40 health";
  s.text(name, tx, y + 4.0f * u, u, (selected ? col::white : tint).alpha(in));
  s.text(desc, tx, y + 20.0f * u, small, (selected ? col::white * 0.9f : col::muted).alpha(in));
  float kw = 18 * u, kh = 18 * u;
  float kx = x + w - kw - 8 * u, ky = y + (h - kh) * 0.5f;
  float line = y + 20.0f * u + 15 * small;
  if (line + 11 * small <= y + h) {
    std::string row;
    for (const std::string &n : numbers()) {
      std::string wider = row.empty() ? n : row + "   " + n;
      if (tx + Screen::textWidth(wider, small) > kx - 6 * u)
        break;
      row = std::move(wider);
    }
    if (!row.empty()) {
      s.text(row, tx, line, small, (selected ? col::xp : col::xp * 0.8f).alpha(in));
      line += 15 * small;
    }
  }
  bool ready = false;
  if (std::string pair = synergy(ready); !pair.empty() && line + 11 * small <= y + h)
    s.text(pair, tx, line, small, (ready ? col::gold : col::muted * 0.75f).alpha(in));

  gui::KeyBadge({kx, ky, kw, kh}, std::to_string(index + 1), tint, u, small, in, selected)
      .render(s);
  renderTag(s, kx - 8 * u, y + 4.0f * u, in);
}
