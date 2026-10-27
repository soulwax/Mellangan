#include "menu/MapMenu.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"

namespace {
constexpr float ZOOM[3] = {2.5f, 5, 10};
} // namespace

void MapMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (in.map.clicked || in.escape.clicked || in.pause.clicked) {
    Sound::select.play(0.5f, 0);
    game.setMenu(nullptr);
    return;
  }
  if (in.confirm.clicked)
    xPan = yPan = 0;
  Key *zooms[3] = {&in.one, &in.two, &in.three};
  for (int i = 0; i < 3; i++)
    if (zooms[i]->clicked)
      zoom = i;
  double speed = 60 * DT / ZOOM[zoom];
  xPan += (in.right.down - in.left.down) * speed;
  yPan += (in.down.down - in.up.down) * speed;
}

void MapMenu::render(Screen &s) {
  Level &l = *game.level;
  Chart &c = l.chart;
  float u = s.u, small = std::max(1.0f, u - 1), pad = 12 * u;
  float legend = Screen::textWidth("THE BLOSSOM GROVE 100%", small) + 16 * u;
  float top = pad + 40 * u, bottom = pad + 22 * small;
  ChartView v{pad,
              top,
              s.w - legend - 3 * pad,
              s.h - top - bottom,
              ZOOM[zoom] * u,
              l.player->x / (16.0 * Chart::CELL) + xPan,
              l.player->y / (16.0 * Chart::CELL) + yPan};

  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.88f});
  s.text("WORLD MAP", pad, pad, 2 * u, col::gold);
  std::string seed = "SEED " + game.worldName();
  s.text(seed, v.x + v.w - Screen::textWidth(seed, small), pad + 16 * u - 8 * small,
         small, col::muted);
  Chart::border(s, v, u);
  c.draw(s, l, v, t);

  float x = v.x + v.w + 2 * pad, y = top, row = 18 * small;
  s.text("BOUNTIES", x, y, small, col::white);
  y += row * 1.5f;
  for (const Bounty &b : l.bounties) {
    s.rect(x, y + 4 * small, 8 * small, 8 * small,
           b.done ? col::gold : col::panel);
    s.frame(x, y + 4 * small, 8 * small, 8 * small, 1, col::muted);
    s.text(b.text(), x + 12 * small, y, small, b.done ? col::gold : col::white);
    if (!b.done && b.goal > 1)
      s.text(fmt::format("{}/{}", std::min(b.progress(l), b.goal), b.goal),
             x + 12 * small, y + row * 0.8f, small, col::muted);
    y += row * (b.done || b.goal <= 1 ? 1.2f : 2);
  }
  y += row;
  s.text("REGIONS", x, y, small, col::white);
  y += row * 1.5f;
  for (int b = 0; b < BIOME_COUNT; b++) {
    if (!c.cells[b])
      continue;
    BiomeId id = BiomeId(b);
    s.rect(x, y + 4 * small, 8 * small, 8 * small, Chart::tone(id));
    Color label = c.knows(id) ? col::white : col::muted;
    s.text(fmt::format("{} {}%", Chart::title(id),
                       c.cells[b] * 100 / std::max(1, c.charted)),
           x + 12 * small, y, small, label);
    y += row;
  }
  s.rect(x, y + 4 * small, 8 * small, 8 * small, {0.3f, 0.32f, 0.3f, 1});
  s.text("EARLIER RUNS", x + 12 * small, y, small, col::muted);
  y += row * 2;
  s.text(fmt::format("{} of {} walked", c.discovered, int(BIOME_COUNT)), x, y,
         small, col::muted);
  y += row;
  s.text(fmt::format("{} tiles charted", c.charted * Chart::CELL * Chart::CELL),
         x, y, small, col::muted);
  y += row * 2;
  for (int k = 0; k < int(StructureKind::Count); k++) {
    s.rect(x + small, y + 5 * small, 6 * small, 6 * small, STRUCTURES[k].debug);
    s.text(STRUCTURES[k].name, x + 12 * small, y, small, col::muted);
    y += row;
  }

  s.text("Arrows pan   1-3 zoom   Enter centre   Tab / Esc close", pad,
         s.h - pad - 16 * small, small, col::muted);
}
