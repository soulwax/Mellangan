#include "menu/MapMenu.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Quests.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"

namespace {
constexpr float ZOOM[3] = {2.5f, 5, 10};
constexpr Color FIRE{1, 0.55f, 0.2f, 1}, MINED{0.65f, 0.6f, 0.55f, 1}, QUEST{1, 0.55f, 0.3f, 1};
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

bool MapMenu::project(const ChartView &v, double wx, double wy, float &sx, float &sy) const {
  sx = v.x + v.w / 2 + float((wx / (16.0 * Chart::CELL) - v.cx) * v.cell);
  sy = v.y + v.h / 2 + float((wy / (16.0 * Chart::CELL) - v.cy) * v.cell);
  return sx > v.x + 2 && sy > v.y + 2 && sx < v.x + v.w - 2 && sy < v.y + v.h - 2;
}

void MapMenu::campaign(Screen &s, const ChartView &v) const {
  Campaign *c = game.campaign.get();
  if (!c)
    return;
  float u = s.u, small = std::max(1.0f, u - 1), sx, sy;
  float pulse = 0.6f + 0.4f * std::sin(t * 4);
  for (const auto &[k, e] : c->edits.changes) {
    int tx = int(int32_t(k >> 32)), ty = int(int32_t(k & 0xFFFFFFFFu));
    if (!e.placed || e.piece != Piece::Campfire || c->founding(tx, ty) ||
        !project(v, tx * 16 + 8.0, ty * 16 + 16.0, sx, sy))
      continue;
    s.rect(sx - 1.5f * u, sy - 1.5f * u, 3 * u, 3 * u, FIRE.alpha(0.9f));
  }
  for (const auto &[rx, ry] : c->delves)
    if (project(v, (rx * 20 + 10) * 16.0, (ry * 20 + 10) * 16.0, sx, sy)) {
      s.line(sx - 3 * u, sy - 3 * u, sx + 3 * u, sy + 3 * u, u, MINED);
      s.line(sx - 3 * u, sy + 3 * u, sx + 3 * u, sy - 3 * u, u, MINED);
    }
  std::vector<Campaign::Town> towns = c->towns();
  for (size_t i = 0; i < towns.size(); i++) {
    if (!project(v, towns[i].x, towns[i].y, sx, sy))
      continue;
    bool home = int(i) + 1 == std::clamp(c->act, 1, int(towns.size()));
    float r = (home ? 5 : 4) * u;
    s.line(sx, sy - r, sx + r, sy, u, col::gold), s.line(sx + r, sy, sx, sy + r, u, col::gold);
    s.line(sx, sy + r, sx - r, sy, u, col::gold), s.line(sx - r, sy, sx, sy - r, u, col::gold);
    if (home)
      s.rect(sx - u, sy - u, 2 * u, 2 * u, col::gold);
    std::string name = ACTS[i].town;
    s.text(name, sx - Screen::textWidth(name, small) / 2, sy + r + 2 * u, small,
           col::gold.alpha(0.9f));
  }
  double qx, qy;
  if (quests::marker(*c, qx, qy) && c->stage != QuestStage::Offered &&
      c->stage != QuestStage::Ready && project(v, qx, qy, sx, sy)) {
    s.ring(sx, sy, 5 * u + 2 * u * pulse, QUEST.alpha(0.5f + 0.5f * pulse));
    s.ring(sx, sy, 2 * u, QUEST);
    std::string label = c->stage == QuestStage::Road ? quests::act(*c).town : "QUEST";
    s.text(label, sx + 8 * u, sy - 6 * small, small, QUEST);
  }
}

float MapMenu::legend(Screen &s, float x, float y) const {
  if (!game.campaign)
    return y;
  float u = s.u, small = std::max(1.0f, u - 1), row = 18 * small;
  s.text("CAMPAIGN", x, y, small, col::white);
  y += row * 1.5f;
  float mx = x + 4 * small, my = y + 8 * small, r = 4 * small;
  s.line(mx, my - r, mx + r, my, 1, col::gold), s.line(mx + r, my, mx, my + r, 1, col::gold);
  s.line(mx, my + r, mx - r, my, 1, col::gold), s.line(mx - r, my, mx, my - r, 1, col::gold);
  s.text("towns and waypoints", x + 12 * small, y, small, col::white);
  y += row;
  s.ring(mx, y + 8 * small, 4 * small, QUEST);
  s.text(Screen::fit(quests::status(*game.campaign), s.w - x - 20 * small, small), x + 12 * small,
         y, small, QUEST);
  y += row;
  s.rect(mx - 1.5f * small, y + 6.5f * small, 3 * small, 3 * small, FIRE);
  s.text("your campfires", x + 12 * small, y, small, col::white);
  y += row;
  s.line(mx - 3 * small, y + 5 * small, mx + 3 * small, y + 11 * small, 1, MINED);
  s.line(mx - 3 * small, y + 11 * small, mx + 3 * small, y + 5 * small, 1, MINED);
  s.text(fmt::format("cleared mines  {}", game.campaign->delves.size()), x + 12 * small, y,
         small, col::white);
  return y + row * 2;
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
  campaign(s, v);

  float x = v.x + v.w + 2 * pad, y = this->legend(s, x, top), row = 18 * small;
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

  s.text(game.input.prompt("Arrows pan   1-3 zoom   Enter centre   Tab / Esc close"), pad,
         s.h - pad - 16 * small, small, col::muted);
}
