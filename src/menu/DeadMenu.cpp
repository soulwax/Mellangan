#include "menu/DeadMenu.hpp"

#include <cmath>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"
#include "menu/Recap.hpp"

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
  float u = s.u, y = s.h * 0.06f;
  Level &l = *game.level;
  s.rect(0, 0, s.w, s.h, {0.05f, 0, 0.02f, std::min(0.6f, t * 0.8f)});
  s.textCenter("YOU HAVE FALLEN", s.w / 2, y, 2.0f * u, col::red);
  float subY = y + 36 * u;
  if (const char *by = l.player->felledBy)
    s.textCenter(fmt::format("felled by {}", by), s.w / 2, subY,
                 std::max(1.0f, u - 1), col::red * 0.85f);
  else
    s.textCenter("overwhelmed by the dark", s.w / 2, subY,
                 std::max(1.0f, u - 1), col::red * 0.85f);

  float nextY = renderEndStats(s, l, subY + 22 * u, std::min(1.0f, t * 1.2f));
  float end = recap(s, l, nextY + 8 * u, std::min(1.0f, t));
  if (t > 1 && std::fmod(t, 1.0f) < 0.65f)
    s.textCenter("R / Enter to rise again", s.w / 2, end + 12 * u, u, col::gold);
  float small = std::max(1.0f, u - 1);
  s.textCenter("world " + game.worldName(), s.w / 2, s.h - 18 * small, small,
               col::muted * 0.8f);
}
