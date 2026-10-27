#include "menu/DeadMenu.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"

void DeadMenu::tick() {
  Menu::tick();
  game.level->tick();
  Input &in = game.input;
  if (t > 1 && (in.restart.clicked || in.confirm.clicked))
    game.newGame();
  else if (in.escape.clicked)
    game.toTitle();
}

void DeadMenu::render(Screen &s) {
  float u = s.u, y = s.h * 0.3f;
  Level &l = *game.level;
  s.rect(0, 0, s.w, s.h, {0.05f, 0, 0.02f, std::min(0.6f, t * 0.8f)});
  s.textCenter("YOU HAVE FALLEN", s.w / 2, y, 3 * u, col::red);
  s.textCenter(fmt::format("Survived {}", clockText(l.time)), s.w / 2,
               y + 64 * u, u, col::white);
  s.textCenter(fmt::format("Level {}   {} kills", l.player->lvl, l.kills),
               s.w / 2, y + 84 * u, u, col::muted);
  if (t > 1 && std::fmod(t, 1.0f) < 0.65f)
    s.textCenter("R / Enter to rise again", s.w / 2, y + 120 * u, u, col::gold);
}
