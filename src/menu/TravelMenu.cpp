#include "menu/TravelMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Quests.hpp"
#include "level/Level.hpp"

void TravelMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  Campaign &c = *game.campaign;
  int n = int(c.towns().size());
  if (in.inventory.clicked || in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(nullptr);
  }
  if (in.up.clicked || in.down.clicked) {
    selected = (selected + in.down.clicked - in.up.clicked + n) % n;
    Sound::select.play(0.25f, 0);
  }
  if (!in.confirm.clicked || t < 0.2f)
    return;
  c.travel(*game.level, selected);
  Sound::potion.play(0.5f, 0, 1.3f);
  game.announce(say(lang.actAnnounce, ACTS[selected].numeral, ACTS[selected].town));
  game.snapCamera();
  game.saveCampaign();
  game.setMenu(nullptr);
}

void TravelMenu::render(Screen &s) {
  Campaign &c = *game.campaign;
  int n = int(c.towns().size());
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  float row = 22 * u, w = 220 * u, h = n * row + 56 * u;
  float x0 = (s.w - w) / 2, y0 = (s.h - h) / 2;
  s.rect(x0, y0, w, h, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, w, h, u);
  s.text(lang.waypoints, x0 + 12 * u, y0 + 8 * u, u, Color{0.45f, 0.85f, 1, 1}.alpha(in));
  for (int i = 0; i < n; i++) {
    float y = y0 + 32 * u + i * row;
    bool on = i == selected;
    s.rect(x0 + 8 * u, y - 3 * u, w - 16 * u, row - 4 * u, col::panel.alpha((on ? 0.95f : 0.6f) * in));
    if (on)
      s.frame(x0 + 8 * u, y - 3 * u, w - 16 * u, row - 4 * u, u, col::gold.alpha(in));
    s.text(say(lang.actRow, ACTS[i].numeral, ACTS[i].town), x0 + 14 * u, y, small,
           (on ? col::gold : col::white).alpha(in));
  }
  s.textCenter(game.input.prompt(lang.travelHint), s.w / 2, y0 + h + 8 * u, small,
               col::muted.alpha(in));
}
