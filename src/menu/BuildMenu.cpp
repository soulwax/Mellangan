#include "menu/BuildMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Harvest.hpp"
#include "entity/Player.hpp"
#include "gui/Tooltip.hpp"
#include "level/Level.hpp"
#include "level/Prop.hpp"

void BuildMenu::tick() {
  Menu::tick();
  toldTime = std::max(0.0f, toldTime - DT);
  Input &in = game.input;
  if (in.build.clicked || in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(nullptr);
  }
  if (in.up.clicked || in.down.clicked) {
    selected = (selected + in.down.clicked - in.up.clicked + BUILDABLE) % BUILDABLE;
    Sound::select.play(0.25f, 0);
  }
  if (!in.confirm.clicked)
    return;
  Player &p = *game.level->player;
  const PieceDef &d = PIECES[selected];
  for (int m = 0; m < MATS; m++)
    if (p.materials[m] < d.cost[m]) {
      told = say(lang.needs, d.cost[m],
                         d.cost[m] == 1 ? MATERIALS[m].name : MATERIALS[m].plural);
      toldTime = 3;
      return Sound::hurt.play(0.3f, 0);
    }
  game.campaign->building = selected;
  Sound::confirm.play(0.5f, 0);
  game.setMenu(nullptr);
}

void BuildMenu::render(Screen &s) {
  Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  float row = 26 * u, w = 260 * u, h = BUILDABLE * row + 70 * u;
  float x0 = (s.w - w) / 2, y0 = (s.h - h) / 2;
  s.rect(x0, y0, w, h, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, w, h, u);
  s.text(lang.build, x0 + 12 * u, y0 + 8 * u, u, col::gold.alpha(in));
  for (int i = 0; i < BUILDABLE; i++) {
    const PieceDef &d = PIECES[i];
    float y = y0 + 34 * u + i * row;
    bool on = i == selected;
    s.rect(x0 + 8 * u, y - 4 * u, w - 16 * u, row - 4 * u, col::panel.alpha((on ? 0.95f : 0.6f) * in));
    if (on)
      s.frame(x0 + 8 * u, y - 4 * u, w - 16 * u, row - 4 * u, u, col::gold.alpha(in));
    Prop prop;
    world::pieceProp(d.piece, 0, 0, prop);
    for (int k = prop.count - 1; k >= 0; k--)
      s.sprite(prop.parts[k].sprite, x0 + 14 * u, y - 3 * u + prop.parts[k].yo * u * 0.5f, 0, 0,
               u * 0.9f, u * 0.9f);
    s.text(d.name, x0 + 36 * u, y, small, col::white.alpha(in));
    gui::materials(s, p.materials, d.cost, x0 + 120 * u, y + 2 * u, small, in);
  }
  if (toldTime > 0)
    s.text(told, x0 + 12 * u, y0 + h - 22 * u, small, col::red.alpha(std::min(1.0f, toldTime) * in));
  s.textCenter(game.input.prompt(lang.buildHint), s.w / 2, y0 + h + 8 * u,
               small, col::muted.alpha(in));
}
