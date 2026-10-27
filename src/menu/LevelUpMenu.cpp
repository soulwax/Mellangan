#include "menu/LevelUpMenu.hpp"

#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "menu/Menu.hpp"

LevelUpMenu::LevelUpMenu(Game &game) : Menu(game) {
  auto &all = game.level->player->upgrades;
  int owned[2] = {0, 0};
  for (auto &u : all)
    if (u->level > 0)
      owned[u->weapon]++;
  for (auto &u : all)
    if (!u->maxed() && (u->level > 0 || owned[u->weapon] < Upgrade::SLOTS))
      choices.push_back(u.get());
  for (size_t i = choices.size(); i > 1; i--)
    std::swap(choices[i - 1], choices[rng.irange(0, int(i) - 1)]);
  if (choices.size() > 3)
    choices.resize(3);
  if (choices.empty())
    choices.push_back(nullptr);
  Sound::levelUp.play(0.6f, 0);
}

std::array<SDL_FRect, 3> LevelUpMenu::cards() const {
  const Screen &s = game.screen;
  float u = s.u, w = std::min(s.w * 0.9f, 340 * u), h = 36 * u, gap = 6 * u;
  float y = s.h * 0.5f - (3 * h + 2 * gap) * 0.5f + 12 * u;
  std::array<SDL_FRect, 3> r{};
  for (int i = 0; i < 3; i++)
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
  Key *keys[] = {&in.one, &in.two, &in.three};
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
  if (Upgrade *u = choices[i]) {
    u->levelUp(p);
    spdlog::info("Level {}: {} -> {}", p.lvl, u->name, u->level);
  } else {
    p.heal(40);
  }
  game.setMenu(--p.pendingLevels > 0 ? std::make_unique<LevelUpMenu>(game)
                                     : nullptr);
}

void LevelUpMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1);
  auto r = cards();
  s.rect(0, 0, s.w, s.h, {0, 0, 0, 0.55f});
  s.textCenter("LEVEL UP!", s.w / 2, r[0].y - 34 * u, 2 * u,
               col::gold.alpha(0.85f + 0.15f * std::sin(t * 6)));

  for (int i = 0; i < int(choices.size()); i++) {
    float in = clampf((t - i * 0.06f) / 0.25f, 0, 1);
    in = 1 - (1 - in) * (1 - in) * (1 - in);
    float x = r[i].x + (1 - in) * 40 * u, y = r[i].y, w = r[i].w, h = r[i].h;
    bool sel = i == selected;
    Upgrade *up = choices[i];

    s.rect(x, y, w, h, col::panel.alpha(0.92f * in));
    if (sel)
      s.rect(x, y, w, h, col::gold.alpha(0.08f * in));
    s.frame(x, y, w, h, u,
            (sel ? col::gold : Color{0.3f, 0.28f, 0.4f, 1}).alpha(in));

    float box = 24 * u, bx = x + 6 * u, by = y + (h - box) / 2;
    s.rect(bx, by, box, box, {0.16f, 0.14f, 0.22f, in});
    s.color(col::white.alpha(in));
    s.sprite(up ? up->icon : atlas::tile(atlas::t::PotionRed), bx + 4 * u,
             by + 4 * u, 0, 0, u, u);
    s.color(col::white);

    float tx = bx + box + 6 * u;
    s.text(up ? up->name : "Roast Feast", tx, y + 5 * u, u,
           col::white.alpha(in));
    s.text(up ? up->desc[up->level] : "Restore 40 health", tx, y + 20 * u,
           small, col::muted.alpha(in));
    if (up) {
      std::string tag =
          up->level == 0 ? "NEW!" : fmt::format("Lv {}", up->level + 1);
      s.text(tag, x + w - Screen::textWidth(tag, small) - 6 * u, y + 5 * u,
             small, (up->level == 0 ? col::gold : col::muted).alpha(in));
    }
  }
  const SDL_FRect &last = r[choices.size() - 1];
  s.textCenter("1-3 / Arrows + Enter / Click", s.w / 2, last.y + last.h + 8 * u,
               small, col::muted);
}
