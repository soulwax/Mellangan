#include "menu/PauseMenu.hpp"

#include "Game.hpp"
#include "menu/CodexMenu.hpp"
#include "menu/SettingsMenu.hpp"
#include "menu/Menu.hpp"

void PauseMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (in.escape.clicked || in.pause.clicked || in.confirm.clicked)
    game.setMenu(nullptr);
  else if (in.quit.clicked)
    game.toTitle();
  else if (in.codex.clicked)
    game.setMenu(std::make_unique<CodexMenu>(game, true));
  else if (in.options.clicked)
    game.setMenu(std::make_unique<SettingsMenu>(game, true));
}

void PauseMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1);
  bool pad = game.input.pad;
  s.rect(0, 0, s.w, s.h, {0, 0, 0, 0.5f});
  s.textCenter("PAUSED", s.w / 2, s.h * 0.35f, 3 * u, col::white);
  s.textCenter(pad ? "A / B to resume" : "Enter / Esc to resume", s.w / 2, s.h * 0.35f + 60 * u, small,
               col::muted);
  s.textCenter(pad ? "Y codex" : "C codex", s.w / 2, s.h * 0.35f + 60 * u + 40 * small, small,
               col::muted);
  s.textCenter(pad ? "RB settings" : "O settings",
               s.w / 2, s.h * 0.35f + 60 * u + 60 * small, small, col::muted);
  s.textCenter(pad ? "Back to quit to title" : "Q to quit to title", s.w / 2, s.h * 0.35f + 60 * u + 20 * small,
               small, col::muted);
  s.textCenter("world " + game.worldName(), s.w / 2, s.h - 30 * small, small,
               col::muted * 0.8f);
}
