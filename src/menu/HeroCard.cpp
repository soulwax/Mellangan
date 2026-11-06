#include "menu/HeroCard.hpp"

#include <string>
#include <vector>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "entity/Player.hpp"
#include "gui/Text.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "menu/HeroSigil.hpp"

namespace {

struct Chip {
  std::string text;
  bool good;
};

std::vector<Chip> chips(const Hero &h) {
  std::vector<Chip> out;
  auto pct = [&](float v, const char *name, bool lowGood) {
    int n = int(std::lround(v * 100));
    if (n)
      out.push_back({fmt::format("{} {:+}%", name, n), lowGood ? n < 0 : n > 0});
  };
  if (h.hp)
    out.push_back({say(lang.chipHp, int(h.hp)), h.hp > 0});
  if (h.armor)
    out.push_back({say(lang.chipArmour, int(h.armor)), true});
  pct(h.speed - 1, lang.chipSpeed, false);
  pct(h.might, lang.chipDamage, false);
  pct(h.haste, lang.chipCooldown, true);
  pct(h.luck, lang.chipCrit, false);
  if (h.regen)
    out.push_back({say(lang.chipRegen, h.regen), true});
  if (h.magnet)
    out.push_back({say(lang.chipReach, int(h.magnet)), true});
  return out;
}

} // namespace

HeroCard::HeroCard(Game &game, const Screen &s, int pick, float t, float in)
    : game(game), h(*HEROES[pick]), pick(pick), t(t), in(in) {
  u = s.u, small = std::max(1.0f, u - 1);
  x = s.w * 0.045f, top = s.h * 0.07f, w = std::min(s.w * 0.34f, 340 * u);
  pad = 12 * u, line = 13 * u, body = u, gut = 22 * u;
  open = h.met(game.codex);
  tint = open ? h.tint : col::muted;
}

const Upgrade *HeroCard::named(const char *name) const {
  return game.level->player->upgrade(name);
}

void HeroCard::render(Screen &s) {
  float tall = layout(s, false);
  if (top + tall > s.h - 30 * u)
    dense = true, tall = layout(s, false);
  s.rect(x, top, w, tall, {0.03f, 0.025f, 0.06f, 0.95f * in});
  s.frame(x, top, w, tall, u, tint.alpha(0.35f * in));
  for (int k = 2; k < 4; k++)
    heroSigil(s, pick, k & 1 ? x + w - 9 * u : x + 9 * u,
              k & 2 ? top + tall - 9 * u : top + 9 * u, 7 * u,
              tint.alpha(0.8f * in), t);
  layout(s, true);
}

float HeroCard::banner(Screen &s, bool draw) const {
  float big = std::min(2 * u, std::floor((w - 46 * u) / Screen::textWidth(h.name, 1)));
  float band = 8 * big + 12 * u;
  if (draw) {
    s.rect(x, top, w, band, tint.alpha(0.12f * in));
    for (int k = 0; k < 6; k++)
      heroSigil(s, pick, x + w - 18 * u - k * 34 * u, top + band / 2, 11 * u,
                tint.alpha(0.07f * in), t);
    heroSigil(s, pick, x + 21 * u, top + band / 2, 14 * u,
              (open ? tint : col::muted * 0.6f).alpha(in), t);
    s.text(tr(h.name), x + 42 * u, top + band / 2 - 8 * big, big, tint.alpha(in));
    s.rect(x, top + band, w, u, tint.alpha(0.7f * in));
  }
  return top + band + 8 * u;
}

float HeroCard::label(Screen &s, const char *text, float y, bool draw) const {
  float cx = x + pad, cw = w - 2 * pad;
  if (draw) {
    s.text(text, cx, y, small, tint.alpha(0.85f * in));
    s.rect(cx + Screen::textWidth(text, small) + 6 * u, y + 8 * small,
           cw - Screen::textWidth(text, small) - 6 * u, 1, tint.alpha(0.25f * in));
  }
  return y + 12 * u;
}

float HeroCard::arsenal(Screen &s, float y, bool draw) const {
  float cx = x + pad, cw = w - 2 * pad;
  float colW = (cw - 6 * u) / 2, step = dense ? 18 * u : 12 * u, iconScale = 0.75f * u;
  int n = 0;
  for (auto &up : game.level->player->upgrades) {
    if (!up->owner || std::string_view(up->owner) != h.name)
      continue;
    float ix = dense ? cx + (n % 8) * step : cx + (n % 2) * (colW + 6 * u),
          iy = y + (dense ? n / 8 : n / 2) * step;
    if (draw) {
      s.sprite(up->icon, ix, iy - u, 0, 0, iconScale, iconScale);
      if (!dense)
        s.text(Screen::fit(tr(up->name), colW - 16 * u, small), ix + 14 * u, iy + u,
               small, (up->weapon ? col::white : tint).alpha(in));
    }
    n++;
  }
  return y + ((n + (dense ? 7 : 1)) / (dense ? 8 : 2)) * step + 6 * u;
}

float HeroCard::nature(Screen &s, float y, bool draw) const {
  float cx = x + pad, cw = w - 2 * pad, chipX = cx;
  for (const Chip &k : chips(h)) {
    float kw = Screen::textWidth(k.text, small) + 10 * u;
    if (chipX + kw > cx + cw)
      chipX = cx, y += 15 * u;
    Color ink = k.good ? tint : col::red * 0.9f;
    if (draw) {
      s.rect(chipX, y, kw, 12 * u, ink.alpha(0.18f * in));
      s.rect(chipX, y, 2 * u, 12 * u, ink.alpha(in));
      s.text(k.text, chipX + 6 * u, y + 2 * u, small, ink.alpha(in));
    }
    chipX += kw + 5 * u;
  }
  y += 15 * u;
  if (draw) {
    s.text(lang.favours, cx, y + 3 * u, small, col::muted.alpha(in));
    float fx = cx + Screen::textWidth(std::string(lang.favours) + " ", small) + 4 * u;
    for (int k = 0; k < 3; k++)
      if (const Upgrade *up = named(h.affinity[k]))
        s.sprite(up->icon, fx + k * 20 * u, y, 0, 0, u, u);
  }
  return y + 18 * u;
}

float HeroCard::layout(Screen &s, bool draw) const {
  float y = banner(s, draw);
  float cx = x + pad, cw = w - 2 * pad;
  Color prose = col::white.alpha(0.85f * in);
  y = gui::wrapText(s, tr(h.title), cx, y, cw, body, line, tint.alpha(0.75f * in), draw) +
      6 * u;
  if (!open) {
    if (draw)
      s.text(lang.lockedClass, cx, y, body, col::red.alpha(in));
    y += line + 4 * u;
    y = gui::wrapText(s, say(lang.unlockNeed, tr(h.need)), cx, y, cw, body, line,
                      prose, draw);
    return y + pad - top;
  }

  y = label(s, lang.cardWeapon, y, draw);
  if (draw) {
    if (const Upgrade *up = named(h.weapon))
      s.sprite(up->icon, cx, y - u, 0, 0, u, u);
    s.text(tr(h.weapon), cx + gut, y, body, col::white.alpha(in));
  }
  y += line + 6 * u;
  y = label(s, lang.cardTrait, y, draw);
  if (draw)
    heroSigil(s, pick, cx + 8 * u, y + 7 * u, 7 * u, tint.alpha(in), t);
  y = gui::wrapText(s, tr(h.trait), cx + gut, y, cw - gut, body, line, prose, draw) + 6 * u;

  y = label(s, lang.cardSkill, y, draw);
  std::string cast = game.input.prompt("SPACE");
  float key = Screen::textWidth(cast, small) + 8 * u;
  if (draw) {
    s.rect(cx + cw - key, y - u, key, 14 * u, tint.alpha(0.15f * in));
    s.frame(cx + cw - key, y - u, key, 14 * u, 1, tint.alpha(0.7f * in));
    s.text(cast, cx + cw - key + 4 * u, y + u, small, tint.alpha(in));
    s.text(tr(h.skill), cx, y, body, tint.alpha(in));
  }
  y += line + 2 * u;
  y = gui::wrapText(s, tr(h.skillDesc), cx, y, cw, body, line, prose, draw) + 6 * u;

  y = arsenal(s, label(s, lang.cardArsenal, y, draw), draw);
  y = nature(s, label(s, lang.cardNature, y, draw), draw);
  return y + pad - top;
}
