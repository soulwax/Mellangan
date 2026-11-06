#include "menu/InventoryMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "gui/Tooltip.hpp"
#include "hero/Hero.hpp"

namespace {

constexpr int SLOTS = GEAR + InventoryMenu::COLS * InventoryMenu::ROWS;

} // namespace

Player &InventoryMenu::player() const { return *game.level->player; }

const Item *InventoryMenu::at(int i) const {
  Player &p = player();
  if (i < GEAR)
    return p.worn[i] ? &p.gear[i] : nullptr;
  size_t k = size_t(i - GEAR);
  return k < p.bag.size() ? &p.bag[k] : nullptr;
}

SDL_FRect InventoryMenu::cell(int i) const {
  const Screen &s = game.screen;
  float u = s.u, size = 22 * u, gap = 3 * u;
  float doll = size + 110 * u, grid = COLS * (size + gap);
  float x0 = (s.w - doll - 20 * u - grid) / 2, y0 = (s.h - 5 * (size + gap)) / 2;
  if (i < GEAR)
    return {x0, y0 + i * (size + gap), size, size};
  int k = i - GEAR;
  return {x0 + doll + 20 * u + (k % COLS) * (size + gap), y0 + (k / COLS) * (size + gap),
          size, size};
}

void InventoryMenu::move(int dx, int dy) {
  if (selected < GEAR) {
    if (dx > 0)
      selected = GEAR + std::min(selected, ROWS - 1) * COLS;
    else
      selected = (selected + dy + GEAR) % GEAR;
    return;
  }
  int k = selected - GEAR, col = k % COLS, row = k / COLS;
  if (dx < 0 && col == 0)
    selected = std::min(row, GEAR - 1);
  else
    selected = GEAR + (row + dy + ROWS) % ROWS * COLS + (col + dx + COLS) % COLS;
}

void InventoryMenu::activate() {
  Player &p = player();
  bool done = selected < GEAR ? p.unequip(selected) : p.equip(size_t(selected - GEAR));
  if (!done)
    return Sound::hurt.play(0.3f, 0);
  (selected < GEAR ? Sound::select : Sound::confirm).play(0.5f, 0);
}

void InventoryMenu::dropSelected() {
  Player &p = player();
  if (!at(selected))
    return;
  Item item;
  if (selected < GEAR) {
    item = p.strip(selected);
  } else {
    item = p.bag[size_t(selected - GEAR)];
    p.bag.erase(p.bag.begin() + (selected - GEAR));
  }
  float a = rng.range(0, TAU);
  game.level->add<ItemDrop>(p.x, p.y, Drop{DropKind::Item, 1, item},
                            p.x + std::cos(a) * 26, p.y + std::sin(a) * 18);
  Sound::select.play(0.4f, 0, 0.7f);
}

void InventoryMenu::tick() {
  Menu::tick();
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
  for (int i = 0; i < SLOTS && (in.mouseMoved || in.mouseClicked); i++) {
    SDL_FRect r = cell(i);
    if (SDL_PointInRectFloat(&m, &r)) {
      selected = i;
      if (in.mouseClicked)
        activate();
    }
  }
  if (in.confirm.clicked)
    activate();
  if (in.erase.clicked)
    dropSelected();
}

void InventoryMenu::render(Screen &s) {
  Player &p = player();
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.8f * in});
  SDL_FRect first = cell(0), last = cell(SLOTS - 1);
  float x0 = first.x - 12 * u, y0 = first.y - 34 * u;
  float w = last.x + last.w - first.x + 24 * u, h = cell(GEAR - 1).y + cell(0).h - y0 + 40 * u;
  s.rect(x0, y0, w, h, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, w, h, u);
  s.text("INVENTORY", x0 + 12 * u, y0 + 10 * u, u, col::gold.alpha(in));

  for (int i = 0; i < SLOTS; i++) {
    SDL_FRect r = cell(i);
    const Item *item = at(i);
    bool on = i == selected;
    s.rect(r.x, r.y, r.w, r.h, col::panel.alpha(0.9f * in));
    Color edge = item ? item->color() : col::muted;
    s.frame(r.x, r.y, r.w, r.h, on ? 2 * u : u,
            (on ? col::gold : edge).alpha((on ? 1.0f : 0.45f) * in));
    if (item)
      s.sprite(item->def().icon, r.x + 3 * u, r.y + 3 * u, 0, 0, u, u);
    if (i < GEAR) {
      float tx = r.x + r.w + 6 * u;
      s.text(gearName(i), tx, r.y + 2 * u, small, col::muted.alpha(in));
      if (item)
        s.text(Screen::fit(item->name(), 100 * u, small), tx, r.y + 2 * u + 16 * small,
               small, item->color().alpha(in));
    }
  }

  float fy = cell(GEAR - 1).y + cell(0).h + 12 * u;
  std::string head = fmt::format("{} gold   bag {}/{}", p.gold, p.bag.size(), Player::BAG);
  s.text(head, x0 + 12 * u, fy, small, col::gold.alpha(0.85f * in));
  gui::materials(s, p.materials, nullptr, x0 + 24 * u + Screen::textWidth(head, small), fy,
                 small, in);
  s.textCenter(game.input.prompt("Arrows or mouse pick   Enter equip   Backspace drop   I close"), s.w / 2,
               y0 + h + 8 * u, small, col::muted.alpha(in));

  if (const Item *item = at(selected)) {
    const Item *worn = nullptr;
    if (selected >= GEAR) {
      int slot = item->gearSlot();
      worn = p.worn[slot] ? &p.gear[slot] : nullptr;
    }
    SDL_FRect r = cell(selected);
    gui::itemTooltip(s, *item, worn, p.hero ? p.hero->name : nullptr, r.x + r.w + 8 * u,
                     r.y);
  }
}
