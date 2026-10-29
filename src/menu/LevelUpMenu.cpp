#include "menu/LevelUpMenu.hpp"

#include <spdlog/spdlog.h>

#include <cstring>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "menu/Menu.hpp"

namespace {
const char *const TYPE_SYMBOLS[6][11] = {
    {"#.........#",
     ".#.......#.",
     "..#.....#..",
     "...#...#...",
     "....#.#....",
     ".....#.....",
     "...#####...",
     "..#..#..#..",
     ".#...#...#.",
     "#....#....#",
     "..........."},
    {"...........",
     ".####.####.",
     "#++++#++++#",
     "#++++#++++#",
     "#++++#++++#",
     "#++++#++++#",
     "#.---+---.#",
     "..#--+--#..",
     "...#---#...",
     "....###....",
     ".....#....."},
    {"...........",
     ".#...#...#.",
     ".#...#...#.",
     ".##..#..##.",
     "#++#+#+#++#",
     "#+++++++++#",
     "#+#+#*#+#+#",
     "#+++++++++#",
     ".#########.",
     "...........",
     "..........."},
    {".....#.....",
     "....#+#....",
     "...#+++#...",
     "..#++-++#..",
     ".#+--#--+#.",
     "....#+#....",
     "...#+++#...",
     "..#++-++#..",
     ".#+--#--+#.",
     "..#--#--#..",
     "..........."},
    {".....#.....",
     "....#+#....",
     "...#+++#...",
     "..#+++++#..",
     ".#+++++++#.",
     "##+++*+++##",
     ".#+++++++#.",
     "..#+++++#..",
     "...#+++#...",
     "....#+#....",
     ".....#....."},
    {"...........",
     ".###...###.",
     "#+++#.#+++#",
     "#++++#++++#",
     "#+++++++++#",
     ".#+++++++#.",
     "..#+++++#..",
     "...#+++#...",
     "....#+#....",
     ".....#.....",
     "..........."}
};

struct UpgradeStyle {
  int symbol;
  Color tierColor;
  bool isClass;
  const char *tierLabel;
};

UpgradeStyle getUpgradeStyle(const Upgrade *up, const Player &p, bool evolving) {
  if (!up)
    return {5, {0.32f, 0.90f, 0.48f, 1.0f}, false, "HEAL"};

  bool isClass = p.hero && ((up->owner && std::strcmp(up->owner, p.hero->name) == 0) ||
                            (p.hero->weapon && std::strcmp(p.hero->weapon, up->name) == 0));

  if (up->rare)
    return {2, {1.0f, 0.58f, 0.16f, 1.0f}, isClass, "RELIC"};

  int nextTier = evolving ? 5 : up->level + 1;
  Color col;
  const char *label = "TIER 1";

  if (nextTier <= 1) {
    col = {0.68f, 0.70f, 0.76f, 1.0f};
    label = "TIER 1";
  } else if (nextTier == 2) {
    col = {0.96f, 0.97f, 1.0f, 1.0f};
    label = "TIER 2";
  } else if (nextTier == 3) {
    col = {0.28f, 0.88f, 0.42f, 1.0f};
    label = "TIER 3";
  } else if (nextTier == 4) {
    col = {0.26f, 0.64f, 0.98f, 1.0f};
    label = "TIER 4";
  } else {
    col = {0.78f, 0.38f, 0.98f, 1.0f};
    label = evolving ? "EVOLVE" : "TIER 5";
  }

  int symbol = evolving ? 3 : isClass ? 4 : up->weapon ? 0 : 1;
  return {symbol, col, isClass, label};
}

void renderSymbol(Screen &s, int index, float x, float y, float cell, Color c, float t) {
  if (index < 0 || index >= 6)
    return;
  const char *const *rows = TYPE_SYMBOLS[index];
  Color light{lerpf(c.r, 1, 0.6f), lerpf(c.g, 1, 0.6f), lerpf(c.b, 1, 0.6f), c.a};
  Color dim = c * 0.55f;
  Color shadow{0.02f, 0.02f, 0.04f, 0.75f * c.a};
  for (int pass = 0; pass < 2; pass++) {
    for (int r = 0; r < 11; r++) {
      for (int col = 0; col < 11; col++) {
        char k = rows[r][col];
        if (k == '.')
          continue;
        float px = x + col * cell, py = y + r * cell;
        if (pass == 0) {
          if (cell > 1)
            s.rect(px + cell, py + cell, cell, cell, shadow);
          continue;
        }
        Color ink = k == '+' ? light : k == '-' ? dim : c;
        if (k == '*')
          ink = light.alpha(c.a * (0.4f + 0.6f * std::abs(std::sin(t * 3.0f + r + col))));
        s.rect(px, py, cell, cell, ink);
      }
    }
  }
}

void renderSilverThread(Screen &s, float x, float y, float w, float h, float u, float in, float t, int index) {
  float ix = x + 2.0f * u, iy = y + 2.0f * u, iw = w - 4.0f * u, ih = h - 4.0f * u;
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

  float phase = t * 0.42f + index * 0.22f;
  float pulse = phase - std::floor(phase);
  float gxTop = ix + 8 * u + pulse * (iw - 16 * u);
  float glowLen = 6 * u;
  s.line(gxTop - glowLen, iy, gxTop + glowLen, iy, u,
         gleam.alpha((0.65f + 0.35f * std::sin(t * 6.0f)) * in));
  s.rect(gxTop - 0.5f * u, iy - 0.5f * u, 2 * u, 2 * u, col::white.alpha(in));

  float gxBot = ix + iw - 8 * u - pulse * (iw - 16 * u);
  s.line(gxBot - glowLen, iy + ih, gxBot + glowLen, iy + ih, u,
         gleam.alpha((0.65f + 0.35f * std::sin(t * 6.0f + 3.14f)) * in));
  s.rect(gxBot - 0.5f * u, iy + ih - 0.5f * u, 2 * u, 2 * u, col::white.alpha(in));
}
}

LevelUpMenu::LevelUpMenu(Game &game) : Menu(game) {
  Player &p = *game.level->player;
  auto &all = p.upgrades;
  if (p.pendingTreasures > 0)
    p.pendingTreasures--, treasure = true;
  int owned[2] = {0, 0};
  for (auto &u : all)
    if (u->level > 0 && !u->rare)
      owned[u->weapon]++;
  std::vector<Upgrade *> relics;
  for (auto &u : all)
    if (u->rare && !u->maxed())
      relics.push_back(u.get());
    else if (!u->rare && !u->maxed() &&
             (u->level > 0 || owned[u->weapon] < Upgrade::SLOTS) &&
             (!u->owner || (p.hero && std::strcmp(u->owner, p.hero->name) == 0)))
      choices.push_back(u.get());
  std::vector<Upgrade *> pool;
  pool.swap(choices);
  auto weight = [&](const Upgrade *u) {
    return u->owner ? 3 : p.hero && p.hero->favours(u->name) ? 2 : 1;
  };
  while (!pool.empty() && choices.size() < 3) {
    int total = 0;
    for (Upgrade *u : pool)
      total += weight(u);
    int r = rng.irange(0, total - 1);
    size_t k = 0;
    while ((r -= weight(pool[k])) >= 0)
      k++;
    choices.push_back(pool[k]);
    pool.erase(pool.begin() + long(k));
  }
  if (choices.empty())
    choices.push_back(nullptr);
  std::vector<Upgrade *> ready;
  for (auto &u : all)
    if (u->canEvolve(p))
      ready.push_back(u.get());
  if (treasure && !ready.empty())
    evolveSlot = int(choices.size()),
    choices.push_back(ready[rng.irange(0, int(ready.size()) - 1)]);
  else if (!relics.empty() && (treasure || rng.chance(0.03f)))
    choices.push_back(relics[rng.irange(0, int(relics.size()) - 1)]);
  Sound::levelUp.play(0.6f, 0);
}

std::array<SDL_FRect, 4> LevelUpMenu::cards() const {
  const Screen &s = game.screen;
  int n = int(choices.size());
  float u = s.u, w = std::min(s.w * 0.90f, 440 * u), h = 46 * u, gap = 6 * u;
  float y = s.h * 0.5f - (n * h + (n - 1) * gap) * 0.5f + 10 * u;
  std::array<SDL_FRect, 4> r{};
  for (int i = 0; i < 4; i++)
    r[i] = {(s.w - w) * 0.5f, y + i * (h + gap), w, h};
  return r;
}

void LevelUpMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  int n = int(choices.size());
  int prev = selected;
  if (in.up.clicked)
    selected = (selected + n - 1) % n;
  if (in.down.clicked)
    selected = (selected + 1) % n;
  Key *keys[] = {&in.one, &in.two, &in.three, &in.four};
  auto r = cards();
  for (int i = 0; i < n; i++) {
    if (keys[i]->clicked)
      return choose(i);
    SDL_FPoint m{in.mouseX, in.mouseY};
    if (!SDL_PointInRectFloat(&m, &r[i]))
      continue;
    if (in.mouseMoved && selected != i)
      selected = i, Sound::select.play(0.4f, 0);
    if (in.mouseClicked && t > 0.3f)
      return choose(i);
  }
  if (selected != prev)
    Sound::select.play(0.4f, 0);
  if (in.confirm.clicked && t > 0.3f)
    choose(selected);
}

void LevelUpMenu::choose(int i) {
  Player &p = *game.level->player;
  Sound::confirm.play(0.6f, 0);
  Upgrade *choice = choices[i];
  if (choice && i == evolveSlot) {
    choice->evolve();
    spdlog::info("Level {}: evolved into {}", p.lvl, choice->name);
  } else if (choice) {
    choice->levelUp(p);
    int relic = 0;
    for (auto &up : p.upgrades)
      if (up->rare && up.get() != choice)
        relic++;
      else if (up.get() == choice)
        break;
    if (choice->rare)
      game.codex.relics |= 1u << relic;
    spdlog::info("Level {}: {} -> {}", p.lvl, choice->name, choice->level);
  } else {
    p.heal(40);
  }
  game.setMenu(--p.pendingLevels > 0 ? std::make_unique<LevelUpMenu>(game)
                                     : nullptr);
}

void LevelUpMenu::render(Screen &s) {
  const Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1);
  auto r = cards();
  s.textCenter(treasure ? "TREASURE!" : "LEVEL UP!", s.w / 2, r[0].y - 28 * u,
               2 * u, col::gold.alpha(0.88f + 0.12f * std::sin(t * 5)));

  for (int i = 0; i < int(choices.size()); i++) {
    float in = clampf((t - i * 0.05f) / 0.22f, 0, 1);
    in = 1 - (1 - in) * (1 - in) * (1 - in);
    float x = r[i].x + (1 - in) * 32 * u, y = r[i].y, w = r[i].w, h = r[i].h;
    bool sel = i == selected;
    Upgrade *up = choices[i];
    bool evolving = i == evolveSlot;
    UpgradeStyle style = getUpgradeStyle(up, p, evolving);
    Color tierCol = style.tierColor;

    s.rect(x, y, w, h, col::panel.alpha(0.92f * in));
    if (sel)
      s.rect(x, y, w, h, tierCol.alpha(0.12f * in));
    s.frame(x, y, w, h, sel ? 2 * u : u, tierCol.alpha((sel ? 1.0f : 0.65f) * in));
    if (sel) {
      float tickLen = 7 * u;
      s.line(x, y, x + tickLen, y, u, col::white.alpha(in));
      s.line(x, y, x, y + tickLen, u, col::white.alpha(in));
      s.line(x + w, y, x + w - tickLen, y, u, col::white.alpha(in));
      s.line(x + w, y, x + w, y + tickLen, u, col::white.alpha(in));
      s.line(x, y + h, x + tickLen, y + h, u, col::white.alpha(in));
      s.line(x, y + h, x, y + h - tickLen, u, col::white.alpha(in));
      s.line(x + w, y + h, x + w - tickLen, y + h, u, col::white.alpha(in));
      s.line(x + w, y + h, x + w, y + h - tickLen, u, col::white.alpha(in));
    }

    if (style.isClass)
      renderSilverThread(s, x, y, w, h, u, in, t, i);

    float badgeSize = 24 * u;
    float bx = x + 6 * u, by = y + (h - badgeSize) * 0.5f;
    s.rect(bx, by, badgeSize, badgeSize,
           {tierCol.r * 0.16f, tierCol.g * 0.16f, tierCol.b * 0.16f, 0.9f * in});
    s.frame(bx, by, badgeSize, badgeSize, u, tierCol.alpha(0.8f * in));
    if (style.isClass)
      s.frame(bx - u, by - u, badgeSize + 2 * u, badgeSize + 2 * u, u,
              Color{0.90f, 0.95f, 1.0f, (0.45f + 0.35f * std::sin(t * 4.0f)) * in});

    renderSymbol(s, style.symbol, bx + (badgeSize - 11 * u) * 0.5f,
                 by + (badgeSize - 11 * u) * 0.5f, u, tierCol.alpha(in), t);

    float iconX = bx + badgeSize + 6 * u;
    float iconY = y + (h - 16 * u) * 0.5f;
    s.color(col::white.alpha(in));
    s.sprite(up ? up->icon : atlas::tile(atlas::t::PotionRed), iconX, iconY, 0, 0,
             u, u);
    s.color(col::white);

    float tx = iconX + 16 * u + 7 * u;
    const char *name = evolving ? up->evolvedName : up ? up->name : "Roast Feast";
    std::string desc = evolving
                           ? fmt::format("{} evolves: double damage, faster", up->name)
                       : up ? up->desc[std::min(up->level, Upgrade::MAX_LEVEL - 1)]
                            : "Restore 40 health";

    s.text(name, tx, y + 5.0f * u, u, (sel ? col::white : tierCol).alpha(in));
    s.text(desc, tx, y + 25.0f * u, small,
           (sel ? col::white * 0.9f : col::muted).alpha(in));

    float kw = 18 * u, kh = 18 * u;
    float kx = x + w - kw - 8 * u, ky = y + (h - kh) * 0.5f;
    s.rect(kx, ky, kw, kh, {0.10f, 0.08f, 0.15f, 0.85f * in});
    s.frame(kx, ky, kw, kh, u, (sel ? tierCol : col::muted * 0.45f).alpha(in));
    s.textCenter(std::to_string(i + 1), kx + kw * 0.5f,
                 ky + (kh - 16 * small) * 0.5f, small,
                 (sel ? col::white : col::muted).alpha(in));

    float tagRight = kx - 8 * u;
    if (evolving) {
      s.text("EVOLVE", tagRight - Screen::textWidth("EVOLVE", small), y + 5.0f * u,
             small, col::xp.alpha(in));
    } else if (up && up->rare) {
      s.text("RELIC", tagRight - Screen::textWidth("RELIC", small), y + 5.0f * u,
             small, tierCol.alpha(in));
    } else if (up && up->level == 0) {
      std::string tagStr = style.isClass
                               ? fmt::format("{}  TIER 1", p.hero ? p.hero->name : "CLASS")
                               : "TIER 1 (NEW)";
      Color tagCol = style.isClass ? Color{0.90f, 0.94f, 1.0f, in} : tierCol.alpha(in);
      s.text(tagStr, tagRight - Screen::textWidth(tagStr, small), y + 5.0f * u,
             small, tagCol);
    } else if (up) {
      std::string tStr = fmt::format("T{}", up->level + 1);
      float tStrW = Screen::textWidth(tStr, small);
      s.text(tStr, tagRight - tStrW, y + 5.0f * u, small, tierCol.alpha(in));

      float pipW = Upgrade::MAX_LEVEL * 4 * u;
      float pipsX = tagRight - tStrW - pipW - 4 * u;
      for (int lvl = 0; lvl < Upgrade::MAX_LEVEL; lvl++) {
        bool lit = lvl <= up->level;
        s.rect(pipsX + lvl * 4 * u, y + 11.0f * u, 2.5f * u, 3.5f * u,
               (lit ? tierCol : Color{0.22f, 0.20f, 0.30f, 1}).alpha(in));
      }
      if (style.isClass) {
        const char *cName = p.hero ? p.hero->name : "CLASS";
        s.text(cName, pipsX - Screen::textWidth(cName, small) - 5 * u, y + 5.0f * u,
               small, Color{0.90f, 0.94f, 1.0f, in});
      }
    }
  }
  const SDL_FRect &last = r[choices.size() - 1];
  s.textCenter(fmt::format("1-{} / Arrows + Enter / Click", choices.size()),
               s.w / 2, last.y + last.h + 9 * u, small, col::muted * 0.75f);
}
