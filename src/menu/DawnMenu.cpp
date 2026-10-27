#include "menu/DawnMenu.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"

void DawnMenu::tick() {
  Menu::tick();
  game.level->tick();
  Input &in = game.input;
  if (t > 2 && (in.restart.clicked || in.confirm.clicked))
    game.newGame();
  else if (in.escape.clicked)
    game.toTitle();
}

void DawnMenu::render(Screen &s) {
  float u = s.u, y = s.h * 0.3f;
  Level &l = *game.level;
  s.rect(0, 0, s.w, s.h, {0.08f, 0.04f, 0.02f, std::min(0.5f, t * 0.4f)});
  s.textCenter("DAWN BREAKS", s.w / 2, y, 3 * u, col::gold);
  s.textCenter(fmt::format("The night ended at {}", clockText(l.time)), s.w / 2,
               y + 64 * u, u, col::white);
  s.textCenter(fmt::format("{} shrine{} kindled   Level {}   {} kills   {} "
                           "regions walked",
                           l.night.lit, l.night.lit == 1 ? "" : "s",
                           l.player->lvl, l.kills, l.chart.discovered),
               s.w / 2, y + 84 * u, u, col::muted);
  if (t > 2 && std::fmod(t, 1.0f) < 0.65f)
    s.textCenter("R / Enter for another night", s.w / 2, y + 120 * u, u,
                 col::gold);
}
