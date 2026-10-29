#include "menu/DawnMenu.hpp"

#include <cmath>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"
#include "menu/Recap.hpp"

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
  float u = s.u, y = s.h * 0.06f;
  Level &l = *game.level;
  s.rect(0, 0, s.w, s.h, {0.08f, 0.04f, 0.02f, std::min(0.5f, t * 0.4f)});
  s.textCenter("DAWN BREAKS", s.w / 2, y, 2.0f * u, col::gold);
  float subY = y + 36 * u;
  s.textCenter(fmt::format("The night was conquered at {}", clockText(l.time)),
               s.w / 2, subY, std::max(1.0f, u - 1), col::gold * 0.85f);

  float nextY = renderEndStats(s, l, subY + 22 * u, std::min(1.0f, t * 0.8f));
  float end = recap(s, l, nextY + 8 * u, std::min(1.0f, t * 0.5f));
  if (t > 2 && std::fmod(t, 1.0f) < 0.65f)
    s.textCenter("R / Enter for another night", s.w / 2, end + 12 * u, u,
                 col::gold);
  float small = std::max(1.0f, u - 1);
  s.textCenter("world " + game.worldName(), s.w / 2, s.h - 18 * small, small,
               col::muted * 0.8f);
}
