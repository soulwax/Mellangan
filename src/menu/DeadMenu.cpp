#include "menu/DeadMenu.hpp"

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
  float u = s.u, y = s.h * 0.2f;
  Level &l = *game.level;
  s.rect(0, 0, s.w, s.h, {0.05f, 0, 0.02f, std::min(0.6f, t * 0.8f)});
  s.textCenter("YOU HAVE FALLEN", s.w / 2, y, 3 * u, col::red);
  if (const char *by = l.player->felledBy)
    s.textCenter(fmt::format("felled by {}", by), s.w / 2, y + 46 * u, u,
                 col::red * 0.8f);
  bool record = l.time > game.best;
  s.textCenter(fmt::format("Survived {}{}", clockText(l.time),
                           record ? "  NEW BEST" : ""),
               s.w / 2, y + 68 * u, u, record ? col::gold : col::white);
  if (l.night.left() > 0)
    s.textCenter(fmt::format("dawn was {} away", clockText(l.night.left())),
                 s.w / 2, y + 104 * u, std::max(1.0f, u - 1), col::muted);
  s.textCenter(fmt::format("Level {}   {} kills   {} region{} walked",
                           l.player->lvl, l.kills, l.chart.discovered,
                           l.chart.discovered == 1 ? "" : "s"),
               s.w / 2, y + 88 * u, u, col::muted);
  float end = recap(s, l, y + 126 * u, std::min(1.0f, t));
  if (t > 1 && std::fmod(t, 1.0f) < 0.65f)
    s.textCenter("R / Enter to rise again", s.w / 2, end + 12 * u, u, col::gold);
  float small = std::max(1.0f, u - 1);
  s.textCenter("world " + game.worldName(), s.w / 2, s.h - 30 * small, small,
               col::muted * 0.8f);
}
