#include "menu/TitleMenu.hpp"

#include "Game.hpp"
#include "Version.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "menu/Menu.hpp"

void TitleMenu::tick() {
    Menu::tick();
    game.level->player->animT += DT;
    if (game.input.confirm.clicked) Sound::confirm.play(0.6f, 0), game.newGame();
    else if (game.input.escape.clicked) game.running = false;
}

void TitleMenu::render(Screen &s) {
    float u = s.u, small = std::max(1.0f, u - 1), y = s.h * 0.12f;
    s.rect(0, 0, s.w, s.h, {0.02f, 0.01f, 0.05f, 0.35f});
    s.textCenter("MELLANGAN", s.w / 2, y, 4 * u, col::gold.alpha(0.9f + 0.1f * std::sin(t * 2)));
    s.textCenter("kindle the shrines, outlast the night", s.w / 2, y + 70 * u, u, col::muted);
    if (std::fmod(t, 1.0f) < 0.65f) s.textCenter("PRESS ENTER", s.w / 2, s.h * 0.62f, u, col::white);
    s.textCenter("WASD/Arrows move   Esc pause   M mute   F11 fullscreen", s.w / 2, s.h - 30 * small, small, col::muted);
    std::string_view version = "v" MELLANGAN_VERSION;
    s.text(version, s.w - Screen::textWidth(version, small) - 6 * small, s.h - 22 * small, small, col::muted * 0.7f);
}
