#include "menu/VendorMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "gui/Tooltip.hpp"
#include "hero/Hero.hpp"
#include "level/Level.hpp"

namespace {

constexpr Color DEAR{1, 0.4f, 0.35f, 1};

std::string wareLabel(const Ware &w) {
  switch (w.kind) {
  case WareKind::Draught:
    return lang.healingDraught;
  case WareKind::Gamble:
    return say(lang.gambleWare, w.item.baseName(true));
  default:
    return w.item.name(true);
  }
}

} // namespace

Player &VendorMenu::player() const { return *game.level->player; }

Vendor &VendorMenu::vendor() const { return game.level->vendor; }

int VendorMenu::wares() const { return int(vendor().wares.size()); }

SDL_FRect VendorMenu::cell(int i) const {
  const Screen &s = game.screen;
  float u = s.u, size = 22 * u, gap = 3 * u, row = 19 * u, list = 200 * u;
  float grid = COLS * (size + gap);
  float x0 = (s.w - list - 20 * u - grid) / 2, y0 = (s.h - 12 * row) / 2;
  int w = wares();
  if (i < w)
    return {x0, y0 + i * row, list, row - 2 * u};
  int k = i - w;
  return {x0 + list + 20 * u + (k % COLS) * (size + gap), y0 + (k / COLS) * (size + gap),
          size, size};
}

void VendorMenu::move(int dx, int dy) {
  int w = wares();
  if (selected < w) {
    if (dx > 0)
      selected = w + std::min(selected, ROWS - 1) * COLS;
    else if (dy)
      selected = (selected + dy + w) % w;
    return;
  }
  int k = selected - w, col = k % COLS, row = k / COLS;
  if (dx < 0 && col == 0)
    selected = std::min(row, w - 1);
  else
    selected = w + (row + dy + ROWS) % ROWS * COLS + (col + dx + COLS) % COLS;
}

void VendorMenu::tell(std::string text, Color c) {
  told = std::move(text), toldColor = c, toldTime = 3;
}

void VendorMenu::activate() {
  Player &p = player();
  Vendor &v = vendor();
  int w = wares();
  if (selected < w) {
    WareKind kind = v.wares[size_t(selected)].kind;
    if (!v.buy(p, size_t(selected), rng))
      return Sound::hurt.play(0.3f, 0);
    if (kind == WareKind::Draught)
      return Sound::potion.play(0.6f, 0);
    if (kind != WareKind::Gamble)
      return Sound::gem.play(0.4f, 0, 1.2f);
    const Item &got = p.bag.back();
    tell(say(lang.gambled, got.name(true)), got.color());
    spdlog::info("gamble: {} [{}] ilvl {}", got.name(), rarityName(got.rarity), got.ilvl);
    if (int(got.rarity) >= int(Rarity::Rare))
      return Sound::chest.play(0.6f, 0, got.rarity == Rarity::Unique ? 0.7f : 1.1f);
    return Sound::confirm.play(0.5f, 0);
  }
  size_t k = size_t(selected - w);
  if (k >= p.bag.size())
    return;
  int gain = Vendor::sellPrice(p.bag[k]);
  Vendor::sell(p, k);
  tell(say(lang.soldFor, gain), col::gold);
  Sound::gem.play(0.35f, 0.05f, 0.9f);
}

void VendorMenu::tick() {
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
      selected = i;
      if (in.mouseClicked)
        activate();
    }
  }
  if (in.confirm.clicked)
    activate();
}

void VendorMenu::render(Screen &s) {
  Player &p = player();
  Vendor &v = vendor();
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  int w = wares();
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.8f * in});
  SDL_FRect first = cell(0), grid = cell(slots() - 1);
  float x0 = first.x - 12 * u, y0 = first.y - 34 * u;
  float pw = grid.x + grid.w - first.x + 24 * u, ph = 12 * 19 * u + 64 * u;
  s.rect(x0, y0, pw, ph, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, pw, ph, u);
  s.text(lang.merchantTitle, x0 + 12 * u, y0 + 10 * u, u, col::gold.alpha(in));
  s.text(lang.freshWares, x0 + 12 * u, y0 + ph - 20 * u, small,
         col::muted.alpha(in));

  for (int i = 0; i < w; i++) {
    const Ware &ware = v.wares[size_t(i)];
    SDL_FRect r = cell(i);
    bool on = i == selected, dim = ware.sold;
    int cost = v.price(ware);
    Color ink = ware.kind == WareKind::Magic ? ware.item.color() : col::white;
    s.rect(r.x, r.y, r.w, r.h, col::panel.alpha((on ? 0.95f : 0.7f) * in));
    if (on)
      s.frame(r.x, r.y, r.w, r.h, u, col::gold.alpha(in));
    Sprite icon = ware.kind == WareKind::Draught ? atlas::tile(atlas::t::PotionRed)
                                                 : ware.item.def().icon;
    s.sprite(icon, r.x + 2 * u, r.y + u, 0, 0, u * 0.9f, u * 0.9f);
    float a = (dim ? 0.35f : 1.0f) * in;
    s.text(Screen::fit(wareLabel(ware), r.w - 80 * u, small), r.x + 20 * u, r.y + 2 * u,
           small, ink.alpha(a));
    std::string tag = dim ? std::string(lang.sold) : fmt::format("{}", cost);
    Color tc = dim ? col::muted : p.gold >= cost ? col::gold : DEAR;
    s.text(tag, r.x + r.w - Screen::textWidth(tag, small) - 4 * u, r.y + 2 * u, small,
           tc.alpha(a));
  }

  for (int i = w; i < slots(); i++) {
    SDL_FRect r = cell(i);
    size_t k = size_t(i - w);
    const Item *item = k < p.bag.size() ? &p.bag[k] : nullptr;
    bool on = i == selected;
    s.rect(r.x, r.y, r.w, r.h, col::panel.alpha(0.9f * in));
    Color edge = item ? item->color() : col::muted;
    s.frame(r.x, r.y, r.w, r.h, on ? 2 * u : u,
            (on ? col::gold : edge).alpha((on ? 1.0f : 0.45f) * in));
    if (item)
      s.sprite(item->def().icon, r.x + 3 * u, r.y + 3 * u, 0, 0, u, u);
  }

  SDL_FRect bag = cell(w);
  s.text(lang.yourBag, bag.x, bag.y - 14 * u, small, col::muted.alpha(in));
  float fy = grid.y + grid.h + 10 * u;
  s.text(say(lang.goldLine, p.gold), bag.x, fy, u, col::gold.alpha(in));
  s.text(say(lang.bagCount, p.bag.size(), Player::BAG), bag.x, fy + 20 * u, small,
         col::muted.alpha(in));
  if (toldTime > 0)
    s.text(Screen::fit(told, grid.x + grid.w - bag.x, small), bag.x, fy + 40 * u, small,
           toldColor.alpha(std::min(1.0f, toldTime) * in));

  std::string hint = game.input.prompt(lang.vendorHint);
  if (selected < w && v.wares[size_t(selected)].kind == WareKind::Gamble)
    hint = lang.gambleHint;
  else if (selected < w && v.wares[size_t(selected)].kind == WareKind::Draught)
    hint = say(lang.draughtHint, Vendor::DRAUGHT);
  s.textCenter(hint, s.w / 2, y0 + ph + 8 * u, small, col::muted.alpha(in));

  const char *hero = p.hero ? p.hero->name : nullptr;
  float tx = bag.x, ty = fy + 60 * u;
  if (selected < w) {
    const Ware &ware = v.wares[size_t(selected)];
    if (ware.kind == WareKind::Draught)
      return;
    int slot = ware.item.gearSlot();
    gui::itemTooltip(s, ware.item, p.worn[slot] ? &p.gear[slot] : nullptr, hero, tx, ty,
                     ware.sold ? -1 : v.price(ware));
  } else if (size_t k = size_t(selected - w); k < p.bag.size()) {
    int slot = p.bag[k].gearSlot();
    gui::itemTooltip(s, p.bag[k], p.worn[slot] ? &p.gear[slot] : nullptr, hero, tx, ty);
  }
}
