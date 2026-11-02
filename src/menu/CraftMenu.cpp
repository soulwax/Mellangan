#include "menu/CraftMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "gui/Tooltip.hpp"
#include "hero/Hero.hpp"
#include "item/craft/Recipes.hpp"
#include "level/Level.hpp"

namespace {

constexpr Color CRAFTED{1, 0.62f, 0.2f, 1}, WORSE{1, 0.4f, 0.35f, 1};

bool stocked(const Player &p, const Formula &r) {
  for (int m = 0; m < MATS; m++)
    if (p.materials[m] < r.cost[m])
      return false;
  return true;
}

} // namespace

Player &CraftMenu::player() const { return *game.level->player; }

const Formula &CraftMenu::chosen() const { return FORMULAS[recipe]; }

int CraftMenu::recipes() const { return FORMULA_COUNT; }

SDL_FRect CraftMenu::cell(int i) const {
  const Screen &s = game.screen;
  float u = s.u, size = 22 * u, gap = 3 * u, row = 17 * u, list = 150 * u;
  float grid = COLS * (size + gap);
  float x0 = (s.w - list - 20 * u - grid) / 2, y0 = (s.h - FORMULA_COUNT * row) / 2;
  int w = recipes();
  if (i < w)
    return {x0, y0 + i * row, list, row - 2 * u};
  int k = i - w;
  return {x0 + list + 20 * u + (k % COLS) * (size + gap), y0 + (k / COLS) * (size + gap),
          size, size};
}

void CraftMenu::move(int dx, int dy) {
  int w = recipes();
  if (selected < w) {
    if (dx > 0)
      selected = w + std::min(selected, ROWS - 1) * COLS;
    else if (dy)
      choose((selected + dy + w) % w);
    return;
  }
  int k = selected - w, col = k % COLS, row = k / COLS;
  if (dx < 0 && col == 0)
    selected = recipe;
  else
    selected = w + (row + dy + ROWS) % ROWS * COLS + (col + dx + COLS) % COLS;
}

void CraftMenu::choose(int r) {
  selected = r;
  if (r != recipe)
    recipe = r, picks.clear();
}

void CraftMenu::tell(std::string text, Color c) {
  told = std::move(text), toldColor = c, toldTime = 3;
}

void CraftMenu::pick(size_t k) {
  Player &p = player();
  const Formula &r = chosen();
  if (k >= p.bag.size())
    return;
  if (auto at = std::find(picks.begin(), picks.end(), k); at != picks.end()) {
    picks.erase(at);
    return Sound::select.play(0.3f, 0, 0.8f);
  }
  if (!r.fits(p.bag[k], r)) {
    tell(fmt::format("{} cannot take that item", r.name), WORSE);
    return Sound::hurt.play(0.3f, 0);
  }
  if (r.inputs == 1)
    picks.clear();
  else if (int(picks.size()) >= r.inputs)
    picks.erase(picks.begin());
  picks.push_back(k);
  Sound::select.play(0.35f, 0, 1.2f);
}

void CraftMenu::activate() {
  int w = recipes();
  if (selected >= w)
    return pick(size_t(selected - w));
  Player &p = player();
  const Formula &r = chosen();
  if (std::string why = missing(p, r, picks); !why.empty()) {
    tell(why, WORSE);
    return Sound::hurt.play(0.3f, 0);
  }
  int at = craft(p, r, picks, rng);
  picks.clear();
  const Item &made = p.bag[size_t(at)];
  tell(fmt::format("{}: {}", r.name, made.name()), made.color());
  spdlog::info("craft: {} -> {} [{}] ilvl {}", r.name, made.name(), rarityName(made.rarity),
               made.ilvl);
  Sound::chest.play(0.5f, 0, made.rarity == Rarity::Crafted ? 0.8f : 1.1f);
  selected = w + at;
}

void CraftMenu::tick() {
  Menu::tick();
  toldTime = std::max(0.0f, toldTime - DT);
  Input &in = game.input;
  if (in.inventory.clicked || in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(nullptr);
  }
  if (in.left.clicked || in.right.clicked || in.up.clicked || in.down.clicked) {
    move(in.right.clicked - in.left.clicked, in.down.clicked - in.up.clicked);
    Sound::select.play(0.25f, 0);
  }
  SDL_FPoint m{in.mouseX, in.mouseY};
  for (int i = 0; i < slots() && (in.mouseMoved || in.mouseClicked); i++) {
    SDL_FRect r = cell(i);
    if (SDL_PointInRectFloat(&m, &r)) {
      if (i < recipes())
        choose(i);
      else
        selected = i;
      if (in.mouseClicked)
        activate();
    }
  }
  if (in.confirm.clicked)
    activate();
}

void CraftMenu::render(Screen &s) {
  Player &p = player();
  const Formula &r = chosen();
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  int w = recipes();
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.8f * in});
  SDL_FRect first = cell(0), grid = cell(slots() - 1), last = cell(w - 1);
  float x0 = first.x - 12 * u, y0 = first.y - 34 * u;
  float pw = grid.x + grid.w - first.x + 24 * u, ph = last.y + last.h - y0 + 30 * u;
  s.rect(x0, y0, pw, ph, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, pw, ph, u);
  s.text("WORKBENCH", x0 + 12 * u, y0 + 10 * u, u, CRAFTED.alpha(in));

  for (int i = 0; i < w; i++) {
    const Formula &rec = FORMULAS[i];
    SDL_FRect c = cell(i);
    bool on = i == recipe;
    s.rect(c.x, c.y, c.w, c.h, col::panel.alpha((on ? 0.95f : 0.7f) * in));
    if (on)
      s.frame(c.x, c.y, c.w, c.h, u, (i == selected ? col::gold : col::muted).alpha(in));
    Color ink = rec.slot >= 0 ? CRAFTED : col::white;
    s.text(rec.name, c.x + 4 * u, c.y + 2 * u, small,
           ink.alpha((stocked(p, rec) ? 1 : 0.4f) * in));
  }

  for (int i = w; i < slots(); i++) {
    SDL_FRect c = cell(i);
    size_t k = size_t(i - w);
    const Item *item = k < p.bag.size() ? &p.bag[k] : nullptr;
    bool on = i == selected, fits = item && r.fits(*item, r),
         picked = std::find(picks.begin(), picks.end(), k) != picks.end();
    s.rect(c.x, c.y, c.w, c.h, col::panel.alpha(0.9f * in));
    Color edge = picked ? CRAFTED : item ? item->color() : col::muted;
    float a = on || picked ? 1.0f : fits ? 0.6f : 0.2f;
    s.frame(c.x, c.y, c.w, c.h, on || picked ? 2 * u : u,
            (on ? col::gold : edge).alpha(a * in));
    if (item) {
      s.color(col::white.alpha((fits || picked ? 1 : 0.3f) * in));
      s.sprite(item->def().icon, c.x + 3 * u, c.y + 3 * u, 0, 0, u, u);
      s.color(col::white);
    }
  }

  SDL_FRect bag = cell(w);
  s.text("YOUR BAG", bag.x, bag.y - 14 * u, small, col::muted.alpha(in));
  float fy = grid.y + grid.h + 10 * u;
  gui::materials(s, p.materials, nullptr, bag.x, fy, u, in);
  std::string head = fmt::format("{}  costs  ", r.name);
  s.text(head, bag.x, fy + 20 * u, small, CRAFTED.alpha(in));
  float cx = bag.x + Screen::textWidth(head, small);
  if (std::all_of(r.cost, r.cost + MATS, [](int c) { return c == 0; }))
    s.text("nothing", cx, fy + 20 * u, small, col::muted.alpha(in));
  else
    gui::materials(s, p.materials, r.cost, cx, fy + 20 * u, small, in);
  std::string why = missing(p, r, picks);
  s.text(why.empty() ? "ready - Enter on the recipe" : why, bag.x, fy + 36 * u, small,
         (why.empty() ? col::gold : col::muted).alpha(in));
  if (toldTime > 0)
    s.text(Screen::fit(told, grid.x + grid.w - bag.x, small), bag.x, fy + 52 * u, small,
           toldColor.alpha(std::min(1.0f, toldTime) * in));

  s.textCenter(fmt::format("{}: {}", r.name, r.text), s.w / 2, y0 + ph + 8 * u, small,
               col::white.alpha(0.85f * in));
  s.textCenter("pick items in the bag, then Enter on the recipe   I close", s.w / 2,
               y0 + ph + 24 * u, small, col::muted.alpha(in));

  if (selected < w)
    return;
  size_t k = size_t(selected - w);
  if (k >= p.bag.size())
    return;
  int slot = p.bag[k].gearSlot();
  gui::itemTooltip(s, p.bag[k], p.worn[slot] ? &p.gear[slot] : nullptr,
                   p.hero ? p.hero->name : nullptr, bag.x, fy + 70 * u);
}
