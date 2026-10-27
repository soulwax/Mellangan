#include "menu/PauseMenu.hpp"

#include "Game.hpp"
#include "menu/Menu.hpp"

void PauseMenu::tick() {
    Menu::tick();
    Input &in = game.input;
    if (in.escape.clicked || in.pause.clicked || in.confirm.clicked) game.setMenu(nullptr);
    else if (in.quit.clicked) game.toTitle();
}

void PauseMenu::render(Screen &s) {
    float u = s.u, small = std::max(1.0f, u - 1);
    s.rect(0, 0, s.w, s.h, {0, 0, 0, 0.5f});
    s.textCenter("PAUSED", s.w / 2, s.h * 0.35f, 3 * u, col::white);
    s.textCenter("Enter / Esc to resume", s.w / 2, s.h * 0.35f + 60 * u, small, col::muted);
    s.textCenter("Q to quit to title", s.w / 2, s.h * 0.35f + 60 * u + 20 * small, small, col::muted);
}
