#include "menu/StashMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "gui/Tooltip.hpp"
#include "hero/Hero.hpp"
#include "level/Level.hpp"

Player &StashMenu::player() const { return *game.level->player; }

std::vector<Item> &StashMenu::stash() const { return game.campaign->stash; }

std::vector<Item> &StashMenu::side(int i) const { return i < CELLS ? player().bag : stash(); }

SDL_FRect StashMenu::cell(int i) const {
  const Screen &s = game.screen;
  float u = s.u, size = 22 * u, gap = 3 * u, grid = COLS * (size + gap), between = 28 * u;
  float x0 = (s.w - 2 * grid - between) / 2, y0 = (s.h - ROWS * (size + gap)) / 2 - 30 * u;
  int k = i % CELLS;
  float x = x0 + (i < CELLS ? 0 : grid + between);
  return {x + (k % COLS) * (size + gap), y0 + (k / COLS) * (size + gap), size, size};
}

void StashMenu::move(int dx, int dy) {
  int half = selected / CELLS, k = selected % CELLS, col = k % COLS, row = k / COLS;
  col += dx;
  if (col < 0 || col >= COLS)
    half = 1 - half, col = (col + COLS) % COLS;
  row = (row + dy + ROWS) % ROWS;
  selected = half * CELLS + row * COLS + col;
}

void StashMenu::activate() {
  bool fromBag = selected < CELLS;
  std::vector<Item> &from = fromBag ? player().bag : stash(),
                    &into = fromBag ? stash() : player().bag;
  size_t k = size_t(selected % CELLS);
  if (k >= from.size())
    return;
  if (int(into.size()) >= (fromBag ? Campaign::STASH : Player::BAG))
    return Sound::hurt.play(0.3f, 0);
  into.push_back(from[k]);
  from.erase(from.begin() + long(k));
  Sound::select.play(0.4f, 0.05f, selected < CELLS ? 0.9f : 1.2f);
}

void StashMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (in.inventory.clicked || in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    game.saveCampaign();
    return game.setMenu(nullptr);
  }
  if (in.left.clicked || in.right.clicked || in.up.clicked || in.down.clicked) {
    move(in.right.clicked - in.left.clicked, in.down.clicked - in.up.clicked);
    Sound::select.play(0.25f, 0);
  }
  SDL_FPoint m{in.mouseX, in.mouseY};
  for (int i = 0; i < 2 * CELLS && (in.mouseMoved || in.mouseClicked); i++) {
    SDL_FRect r = cell(i);
    if (SDL_PointInRectFloat(&m, &r)) {
      selected = i;
      if (in.mouseClicked)
        activate();
    }
  }
  if (in.confirm.clicked)
    activate();
}

void StashMenu::render(Screen &s) {
  Player &p = player();
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.8f * in});
  SDL_FRect first = cell(0), last = cell(2 * CELLS - 1);
  float x0 = first.x - 12 * u, y0 = first.y - 40 * u;
  float pw = last.x + last.w - first.x + 24 * u, ph = last.y + last.h - y0 + 30 * u;
  s.rect(x0, y0, pw, ph, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, pw, ph, u);
  s.text(lang.stash, x0 + 12 * u, y0 + 8 * u, u, col::gold.alpha(in));
  for (int i = 0; i < 2 * CELLS; i++) {
    SDL_FRect r = cell(i);
    std::vector<Item> &items = side(i);
    size_t k = size_t(i % CELLS);
    const Item *item = k < items.size() ? &items[k] : nullptr;
    bool on = i == selected;
    s.rect(r.x, r.y, r.w, r.h, col::panel.alpha(0.9f * in));
    Color edge = item ? item->color() : col::muted;
    s.frame(r.x, r.y, r.w, r.h, on ? 2 * u : u,
            (on ? col::gold : edge).alpha((on ? 1.0f : 0.45f) * in));
    if (item)
      s.sprite(item->def().icon, r.x + 3 * u, r.y + 3 * u, 0, 0, u, u);
  }
  SDL_FRect bag = cell(0), chest = cell(CELLS);
  s.text(say(lang.yourBagCount, p.bag.size(), Player::BAG), bag.x, bag.y - 14 * u, small,
         col::muted.alpha(in));
  s.text(say(lang.stashCount, stash().size(), Campaign::STASH), chest.x, chest.y - 14 * u,
         small, col::muted.alpha(in));
  s.textCenter(game.input.prompt(lang.stashHint), s.w / 2,
               y0 + ph + 8 * u, small, col::muted.alpha(in));
  std::vector<Item> &items = side(selected);
  size_t k = size_t(selected % CELLS);
  if (k >= items.size())
    return;
  int slot = items[k].gearSlot();
  gui::itemTooltip(s, items[k], p.worn[slot] ? &p.gear[slot] : nullptr,
                   p.hero ? p.hero->name : nullptr, first.x, last.y + last.h + 30 * u);
}
