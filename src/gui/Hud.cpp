#include "gui/Hud.hpp"

#include "Game.hpp"
#include <spdlog/spdlog.h>
#include <cctype>
#include <cstdio>
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "gfx/StatusVisual.hpp"
#include "entity/enemy/Enemy.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "level/Flavour.hpp"
#include "level/Noise.h"
#include "menu/DawnMenu.hpp"
#include "menu/DeadMenu.hpp"
#include "menu/LevelUpMenu.hpp"
#include "menu/MapMenu.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/ShrineMenu.hpp"
#include "menu/TitleMenu.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "level/World.hpp"

void Hud::announce(std::string text) { banners.push_back(std::move(text)); }

void Hud::clear() {
  bannerTime = 0;
  banners.clear();
}

void Hud::tick(bool running) {
  if (running)
    bannerTime = std::max(0.0f, bannerTime - DT);
  if (bannerTime <= 0 && !banners.empty())
    banner = std::move(banners.front()), banners.pop_front(), bannerTime = 3;
}

void Hud::render(float time, int xView, int yView) {
  this->time = time, this->xView = xView, this->yView = yView;
  draw();
}

bool Hud::edge(double wx, double wy, float &xe, float &ye, float &xn,
                float &yn) const {
  const Screen &s = game.screen;
  float k = s.scale, u = s.u;
  float xo = std::floor((s.w - s.vw * k) / 2),
        yo = std::floor((s.h - s.vh * k) / 2);
  float xm = xo + s.vw * k * 0.5f, ym = yo + s.vh * k * 0.5f;
  float xd = xo + float(wx - xView) * k - xm, yd = yo + float(wy - yView) * k - ym,
        dd = std::max(1.0f, std::sqrt(xd * xd + yd * yd));
  float xh = s.vw * k * 0.5f - 18 * u,
        yh = s.vh * k * 0.5f - (yd < 0 ? 84 : game.level->bossIds.empty() ? 18 : 46) * u;
  float fit = std::min({1.0f, xd != 0 ? xh / std::abs(xd) : 1e9f,
                        yd != 0 ? yh / std::abs(yd) : 1e9f});
  xe = xm + xd * fit, ye = ym + yd * fit, xn = xd / dd, yn = yd / dd;
  if (fit >= 1)
    xn = 0, yn = 1;
  return fit < 1;
}

void Hud::landmarks() {
  if (game.menu)
    return;
  Player &p = *game.level->player;
  int rx = world::regionOf(int(std::floor(p.x / 16))),
      ry = world::regionOf(int(std::floor(p.y / 16)));
  Structure best;
  double bestD = 1e18;
  for (int y = ry - 4; y <= ry + 4; y++)
    for (int x = rx - 4; x <= rx + 4; x++) {
      Structure site;
      if (!world::structureIn(x, y, site) || !site.landmark() ||
          game.level->visited(site))
        continue;
      double xd = site.centerX() - p.x, yd = site.centerY() - p.y;
      if (xd * xd + yd * yd < bestD)
        bestD = xd * xd + yd * yd, best = site;
    }
  double tiles = std::sqrt(bestD) / 16, bx = best.centerX(),
         by = (best.y + 5) * 16.0;
  if (bx > xView - 24 && bx < xView + game.screen.vw + 24 && by > yView &&
      by - 64 < yView + game.screen.vh)
    return;
  float xe, ye, xn, yn;
  if (tiles > 72 || !edge(best.centerX(), best.centerY() - 24, xe, ye, xn, yn))
    return;
  Screen &s = game.screen;
  float u = s.u, small = std::max(1.0f, u - 1);
  float fade = clampf(float((72 - tiles) / 24), 0, 1) * 0.8f;
  Color c = best.debugColor().alpha(fade * (0.8f + 0.2f * std::sin(time * 2)));
  float r = 4 * u;
  for (int i = 0; i < 4; i++) {
    float a = float(TAU / 4 * i), b = float(TAU / 4 * (i + 1));
    s.line(xe + std::cos(a) * r, ye + std::sin(a) * r, xe + std::cos(b) * r,
           ye + std::sin(b) * r, u, c);
  }
  std::string tag = fmt::format("{} {:.0f}", best.name(), tiles);
  float half = Screen::textWidth(tag, small) * 0.5f + 4 * u;
  s.textCenter(tag, clampf(xe - xn * 16 * u, half, s.w - half),
               ye - yn * 16 * u - 8 * small, small, c.alpha(c.a * 0.85f));
}

void Hud::champions() {
  if (game.menu)
    return;
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1);
  for (entt::entity id : game.level->bossIds) {
    Enemy *e = game.level->get<Enemy>(id);
    if (!e || e->removed || !e->title || !s.visible(e->x, e->y, 600) ||
        s.visible(e->x, e->y, 8))
      continue;
    float xe, ye, xn, yn;
    if (!edge(e->x, e->bodyY(), xe, ye, xn, yn))
      continue;
    Color c = col::gold.alpha(0.7f + 0.3f * std::sin(time * 6));
    s.line(xe, ye, xe - xn * 7 * u - yn * 4 * u, ye - yn * 7 * u + xn * 4 * u, 2 * u, c);
    s.line(xe, ye, xe - xn * 7 * u + yn * 4 * u, ye - yn * 7 * u - xn * 4 * u, 2 * u, c);
    double xs = e->x - p.x, ys = e->y - p.y;
    std::string tag = fmt::format("{} {:.0f}", e->title, std::sqrt(xs * xs + ys * ys) / 16);
    float half = Screen::textWidth(tag, small) * 0.5f + 4 * u;
    s.textCenter(tag, clampf(xe - xn * 18 * u, half, s.w - half),
                 ye - yn * 18 * u - 8 * small, small, c);
  }
}

void Hud::compass() {
  Night &n = game.level->night;
  if (!n.hasBeacon || game.menu)
    return;
  Screen &s = game.screen;
  float u = s.u, small = std::max(1.0f, u - 1), xe, ye, xn, yn;
  edge(n.beacon.centerX(), n.beacon.centerY() - 24, xe, ye, xn, yn);
  Color c = col::gold.alpha(0.75f + 0.25f * std::sin(time * 5));
  s.line(xe, ye, xe - xn * 8 * u - yn * 5 * u, ye - yn * 8 * u + xn * 5 * u,
         2 * u, c);
  s.line(xe, ye, xe - xn * 8 * u + yn * 5 * u, ye - yn * 8 * u - xn * 5 * u,
         2 * u, c);
  Player &p = *game.level->player;
  double xs = n.beacon.centerX() - p.x, ys = n.beacon.centerY() - p.y;
  std::string tag =
      fmt::format("SHRINE {:.0f}", std::sqrt(xs * xs + ys * ys) / 16);
  float half = Screen::textWidth(tag, small) * 0.5f + 4 * u;
  s.textCenter(tag, clampf(xe - xn * 20 * u, half, s.w - half),
               ye - yn * 20 * u - 8 * small, small, c);
}

void Hud::minimap() {
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1), side = 64 * u;
  float x = s.w - side - 4 * u, y = s.h - side - 6 * u;
  ChartView v{x,
              y,
              side,
              side,
              2 * u,
              p.x / (16.0 * Chart::CELL),
              p.y / (16.0 * Chart::CELL)};
  Chart::border(s, v, u);
  game.level->chart.draw(s, *game.level, v, time);
  BiomeId b =
      world::biomeAt(int(std::floor(p.x / 16)), int(std::floor(p.y / 16)));
  std::string_view name = Chart::title(b), note = FLAVOURS[b].note;
  s.text(note, s.w - Screen::textWidth(note, small) - 4 * u,
         y - 4 * u - 32 * small, small, col::gold * 0.8f);
  s.text(name, s.w - Screen::textWidth(name, small) - 4 * u,
         y - 2 * u - 16 * small, small, col::muted);
  std::string_view sky = game.level->sky.name();
  s.text(sky, s.w - Screen::textWidth(sky, small) - 4 * u,
         y - 6 * u - 48 * small, small, Color{0.62f, 0.72f, 0.9f, 0.85f});
}

void Hud::draw() {
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1), bar = 6 * u, top = bar + 5 * u;

  s.rect(0, 0, s.w, bar + 2 * u, col::panel);
  float fill = (s.w - 2 * u) * clampf(p.exp / p.expNext, 0, 1);
  s.rect(u, u, fill, bar, col::xp);
  s.rect(u, u, fill, u, {0.75f, 0.88f, 1, 0.6f});

  std::string lv = fmt::format("LV {}", p.lvl),
              kills = fmt::format("{} kills", game.level->kills);
  s.text(lv, s.w - Screen::textWidth(lv, u) - 4 * u, top, u, col::white);
  s.text(kills, s.w - Screen::textWidth(kills, small) - 4 * u, top + 18 * u,
         small, col::muted);
  if (game.level->streak >= 10) {
    std::string streak = fmt::format("x{} STREAK", game.level->streak);
    s.text(streak, s.w - Screen::textWidth(streak, small) - 4 * u, top + 34 * u,
           small,
           col::gold.alpha(0.5f + 0.5f * std::min(1.0f, game.level->streakTime)));
  }
  Night &n = game.level->night;
  s.textCenter(fmt::format("DAWN {}", clockText(n.left())), s.w / 2, top,
               std::round(u * 1.5f), col::white);
  if (game.level->omen >= 0)
    s.textCenter(OMENS[game.level->omen].name, s.w / 2, top + 58 * u, small,
                 OMENS[game.level->omen].tint);
  s.textCenter(fmt::format("{} shrine{} lit", n.lit, n.lit == 1 ? "" : "s"),
               s.w / 2, top + 26 * u, small, n.lit ? col::gold : col::muted);
  if (n.kindling > 0) {
    float kw = 120 * u, kx = (s.w - kw) / 2, ky = top + 46 * u;
    s.rect(kx, ky, kw, 4 * u, col::panel);
    s.rect(kx, ky, kw * n.kindling, 4 * u, col::gold);
  }
  landmarks();
  champions();
  compass();
  minimap();

  int slot[2] = {0, 0};
  for (auto &up : p.upgrades) {
    if (!up->level)
      continue;
    int row = up->weapon ? 0 : 1;
    float x = 4 * u + slot[row]++ * 20 * u, y = top + row * 24 * u;
    s.rect(x, y, 18 * u, 18 * u, {0.07f, 0.06f, 0.11f, 0.7f});
    s.sprite(up->icon, x + u, y + u, 0, 0, u, u);
    if (up->rare || up->evolved) {
      s.frame(x, y, 18 * u, 18 * u, u,
              (up->evolved ? col::xp : col::gold).alpha(0.85f));
      continue;
    }
    for (int i = 0; i < up->level; i++)
      s.rect(x + u + i * 3.4f * u, y + 19 * u, 2 * u, 2 * u, col::gold);
  }

  float sx = 4 * u, sy = top + 54 * u;
  for (int i = 0; i < int(StatusKind::Count); i++) {
    StatusKind kind = StatusKind(i);
    const StatusEffect &effect = p.status.effects[i];
    if (effect.time <= 0)
      continue;
    std::string label = fmt::format("{} {:.1f}s", statusName(kind), effect.time);
    s.text(label, sx, sy, small, statusColor(kind));
    sx += Screen::textWidth(label, small) + 8 * u;
  }

  auto &ids = game.level->bossIds;
  ids.erase(std::remove_if(ids.begin(), ids.end(),
                           [&](entt::entity id) {
                             Enemy *e = game.level->get<Enemy>(id);
                             return !e || e->removed;
                           }),
            ids.end());
  if (!ids.empty() && !p.dead()) {
    Enemy *top = game.level->get<Enemy>(ids.front());
    for (entt::entity id : ids)
      if (Enemy *e = game.level->get<Enemy>(id); e->def && !top->def)
        top = e;
    Enemy &b = *top;
    const char *name = b.name();
    float bw = 160 * u, bx = (s.w - bw) / 2, by = s.h - 22 * u;
    float k = clampf(b.hp / b.maxHp, 0, 1);
    s.textCenter(ids.size() > 1 ? fmt::format("{}  +{}", name, ids.size() - 1)
                                : std::string(name),
                 s.w / 2, by - 15 * small, small, Color{1, 0.45f, 0.45f, 1});
    s.rect(bx - u, by - u, bw + 2 * u, 6 * u, col::panel);
    s.rect(bx, by, bw * k, 4 * u, Color{0.85f, 0.12f, 0.18f, 1});
    s.rect(bx, by, bw * k, u, Color{1, 0.5f, 0.5f, 0.6f});
    if (b.def)
      for (int i = 1; i < 3; i++)
        s.rect(bx + bw * i / 3 - u * 0.5f, by - u, u, 6 * u,
               (i > 2 - b.phase ? col::muted : col::white).alpha(0.8f));
  }
  float low = 1 - clampf(p.hp / (p.maxHp * 0.3f), 0, 1);
  if (low > 0 && !p.dead()) {
    float rate = 4 + 6 * low, beat = std::pow(std::abs(std::sin(time * rate)), 6.0f);
    float a = (0.12f + 0.28f * beat) * low;
    for (int i = 0; i < 4; i++) {
      float band = (4 - i) * 6 * u, k = a * (i + 1) / 4;
      Color c = col::red.alpha(k * 0.6f);
      s.rect(0, 0, s.w, band, c);
      s.rect(0, s.h - band, s.w, band, c);
      s.rect(0, band, band, s.h - 2 * band, c);
      s.rect(s.w - band, band, band, s.h - 2 * band, c);
    }
  }
  float hw = 80 * u, hy = s.h - 12 * u;
  if (const Hero *h = p.hero) {
    float box = 22 * u, bx = 4 * u, by = hy - box - 8 * u;
    const Skill *sk = p.skill;
    float wait = p.skillTime /
                 std::max(0.01f, (sk ? sk->cooldown : h->cooldown) * p.haste);
    s.rect(bx, by, box, box, col::panel);
    s.sprite(sk ? sk->icon : h->sprite(), bx + 3 * u, by + 3 * u, 0, 0, u, u);
    if (wait > 0)
      s.rect(bx, by, box, box * wait, {0.02f, 0.02f, 0.05f, 0.7f});
    s.frame(bx, by, box, box, u,
            wait > 0 ? col::muted.alpha(0.4f) : h->tint.alpha(0.9f));
    s.text(sk ? sk->name : h->skill, bx + box + 5 * u, by + 2 * u, small,
           wait > 0 ? col::muted : h->tint);
    s.text(wait > 0 ? fmt::format("{:.0f}s", std::ceil(p.skillTime))
                    : std::string("SPACE"),
           bx + box + 5 * u, by + 2 * u + 16 * small, small, col::muted * 0.8f);
  }
  s.rect(4 * u, hy, hw, 6 * u, col::panel);
  s.rect(5 * u, hy + u, (hw - 2 * u) * clampf(p.hp / p.maxHp, 0, 1), 4 * u,
         col::red);
  s.text(fmt::format("{}/{}", int(std::ceil(p.hp)), int(p.maxHp)), 8 * u + hw,
         hy - 5 * small, small, col::white);

  if (bannerTime > 0 && !game.menu) {
    float a = std::min({1.0f, bannerTime * 2, (3 - bannerTime) * 4});
    float size = u, y = s.h * 0.22f;
    float pw = Screen::textWidth(banner, size) + 14 * u, ph = 16 * size + 6 * u;
    float px = std::floor(s.w / 2 - pw / 2), py = y - 2 * u;
    s.rect(px, py, pw, ph, col::panel.alpha(0.72f * a));
    s.rect(px, py, pw, u, col::gold.alpha(0.55f * a));
    s.rect(px, py + ph - u, pw, u, col::gold.alpha(0.55f * a));
    s.textCenter(banner, s.w / 2, y, size, col::gold.alpha(a));
  }
}
