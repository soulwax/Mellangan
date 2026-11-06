#include "gui/Hud.hpp"

#include "Game.hpp"
#include "gui/Lang.hpp"
#include "entity/Merchant.hpp"
#include "entity/Workbench.hpp"
#include "campaign/Harvest.hpp"
#include "campaign/Elder.hpp"
#include "campaign/Quests.hpp"
#include "campaign/Townsfolk.hpp"
#include "campaign/Sentry.hpp"
#include "level/Prop.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "gfx/StatusVisual.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "item/skill/Skill.hpp"
#include "level/Flavour.hpp"
#include "level/World.hpp"

bool Hud::edge(double wx, double wy, Pin &pin) const {
  const Screen &s = game.screen;
  float k = s.scale, u = s.u;
  float xo = std::floor((s.w - s.vw * k) / 2),
        yo = std::floor((s.h - s.vh * k) / 2);
  float xm = xo + s.vw * k * 0.5f, ym = yo + s.vh * k * 0.5f;
  float xd = xo + float(wx - xView) * k - xm, yd = yo + float(wy - yView) * k - ym,
        dd = std::max(1.0f, std::sqrt(xd * xd + yd * yd));
  float xh = std::min(s.vw * k, s.w) * 0.5f - 18 * u,
        yh = std::min(s.vh * k, s.h) * 0.5f - (yd < 0 ? 84 : game.level->bossIds.empty() ? 18 : 46) * u;
  float fit = std::min({1.0f, xd != 0 ? xh / std::abs(xd) : 1e9f,
                        yd != 0 ? yh / std::abs(yd) : 1e9f});
  pin = {xm + xd * fit, ym + yd * fit, xd / dd, yd / dd};
  if (fit >= 1)
    pin.nx = 0, pin.ny = 1;
  return fit < 1;
}

SDL_FPoint Hud::gui(double wx, double wy) const {
  const Screen &s = game.screen;
  float k = s.scale;
  return {std::floor((s.w - s.vw * k) / 2) + float(wx - xView) * k,
          std::floor((s.h - s.vh * k) / 2) + float(wy - yView) * k};
}

void Hud::plate(double wx, double wy, std::string_view text, Color ink) {
  Screen &s = game.screen;
  SDL_FPoint g = gui(wx, wy);
  float u = s.u, small = std::max(1.0f, u - 1);
  float w = Screen::textWidth(text, small) + 6 * u, h = 16 * small + 2 * u;
  s.rect(g.x - w / 2, g.y - h, w, h, col::panel.alpha(0.82f));
  s.text(text, g.x - w / 2 + 3 * u, g.y - h + u, small, ink);
}

bool Hud::inView(double wx, double wy, float margin) const {
  const Screen &s = game.screen;
  double xv = wx - xView, yv = wy - yView;
  return xv > -margin && yv > -margin && xv < s.vw + margin && yv < s.vh + margin;
}

void Hud::arrow(const Pin &pin, float length, float wing, Color c) {
  Screen &s = game.screen;
  float u = s.u, xe = pin.x, ye = pin.y, xn = pin.nx, yn = pin.ny;
  s.line(xe, ye, xe - xn * length * u - yn * wing * u,
         ye - yn * length * u + xn * wing * u, 2 * u, c);
  s.line(xe, ye, xe - xn * length * u + yn * wing * u,
         ye - yn * length * u - xn * wing * u, 2 * u, c);
}

void Hud::diamond(const Pin &pin, Color c) {
  Screen &s = game.screen;
  float u = s.u, r = 4 * u;
  for (int i = 0; i < 4; i++) {
    float a = float(TAU / 4 * i), b = float(TAU / 4 * (i + 1));
    s.line(pin.x + std::cos(a) * r, pin.y + std::sin(a) * r,
           pin.x + std::cos(b) * r, pin.y + std::sin(b) * r, u, c);
  }
}

void Hud::tag(const std::string &text, const Pin &pin, float back, Color c) {
  Screen &s = game.screen;
  float u = s.u, small = std::max(1.0f, u - 1);
  float half = Screen::textWidth(text, small) * 0.5f + 4 * u, h = 16 * small + 2 * u;
  float x = clampf(pin.x - pin.nx * back * u, half, s.w - half),
        y = pin.y - pin.ny * back * u - 8 * small;
  float step = pin.y > s.h * 0.5f ? -h : h;
  for (int tries = 0; tries < 8; tries++) {
    bool clear = true;
    for (const SDL_FRect &r : tags)
      clear &= x + half <= r.x || x - half >= r.x + r.w || y + h <= r.y || y >= r.y + r.h;
    if (clear)
      break;
    y += step;
  }
  tags.push_back({x - half, y, 2 * half, h});
  s.textCenter(text, x, y, small, c);
}

void Hud::landmarks() {
  if (game.menu)
    return;
  Player &p = *game.level->player;
  int rx = world::regionOf(world::tileOf(p.x)), ry = world::regionOf(world::tileOf(p.y));
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
  if (bestD >= 1e18)
    return;
  double tiles = std::sqrt(bestD) / 16, bx = best.centerX(),
         by = (best.y + 5) * 16.0;
  if (bx > xView - 24 && bx < xView + game.screen.vw + 24 && by > yView &&
      by - 64 < yView + game.screen.vh)
    return;
  Pin pin;
  if (tiles > 72 || !edge(best.centerX(), best.centerY() - 24, pin))
    return;
  float fade = clampf(float((72 - tiles) / 24), 0, 1) * 0.8f;
  Color c = best.debugColor().alpha(fade * (0.8f + 0.2f * std::sin(time * 2)));
  diamond(pin, c);
  tag(fmt::format("{} {:.0f}", best.name(), tiles), pin, 16, c.alpha(c.a * 0.85f));
}

void Hud::champions() {
  if (game.menu)
    return;
  Player &p = *game.level->player;
  for (entt::entity id : game.level->bossIds) {
    Enemy *e = game.level->get<Enemy>(id);
    if (!e || e->removed || !e->title || !inView(e->x, e->y, 600) || inView(e->x, e->y, 8))
      continue;
    Pin pin;
    if (!edge(e->x, e->bodyY(), pin))
      continue;
    Color c = col::gold.alpha(0.7f + 0.3f * std::sin(time * 6));
    arrow(pin, 7, 4, c);
    double xs = e->x - p.x, ys = e->y - p.y;
    tag(fmt::format("{} {:.0f}", tr(e->title), std::sqrt(xs * xs + ys * ys) / 16), pin, 18, c);
  }
}

void Hud::merchant() {
  Level &l = *game.level;
  Merchant *m = l.get<Merchant>(l.vendor.merchant);
  if (!m || game.menu)
    return;
  Player &p = *l.player;
  double xs = m->x - p.x, ys = m->y - p.y, tiles = std::sqrt(xs * xs + ys * ys) / 16;
  Color c = Color{0.95f, 0.75f, 0.4f, 1};
  if (!inView(m->x, m->y, -8)) {
    Pin pin;
    if (tiles < 48 && edge(m->x, m->y - 10, pin)) {
      diamond(pin, c.alpha(0.7f));
      tag(say(lang.merchantAt, tiles), pin, 16, c.alpha(0.7f));
    }
    return;
  }
  auto sign = [&](const Entity &e, std::string_view text, bool close, float rise) {
    plate(e.x, e.y - rise, text,
          close ? col::gold.alpha(0.75f + 0.25f * std::sin(time * 6)) : c.alpha(0.8f));
  };
  bool close = Merchant::near(l);
  sign(*m, close ? game.input.prompt(lang.trade) : lang.merchant, close, 24);
  if (Workbench *b = Workbench::near(l); b && !close)
    sign(*b, game.input.prompt(lang.craft), true, 18);
}

void Hud::compass() {
  Night &n = game.level->night;
  if (!n.hasBeacon || game.menu)
    return;
  Pin pin;
  edge(n.beacon.centerX(), n.beacon.centerY() - 24, pin);
  Color c = col::gold.alpha(0.75f + 0.25f * std::sin(time * 5));
  arrow(pin, 8, 5, c);
  Player &p = *game.level->player;
  double xs = n.beacon.centerX() - p.x, ys = n.beacon.centerY() - p.y;
  tag(say(lang.shrineAt, std::sqrt(xs * xs + ys * ys) / 16), pin, 20, c);
}

void Hud::lootLabels() {
  Screen &s = game.screen;
  float k = s.scale, u = s.u, small = std::max(1.0f, u - 1);
  float xo = std::floor((s.w - s.vw * k) / 2), yo = std::floor((s.h - s.vh * k) / 2);
  bool held = game.input.loot.down;
  struct Tag {
    float x, y, w, h;
    std::string text;
    Color c;
  };
  std::vector<Tag> labels;
  game.level->each<ItemDrop>([&](ItemDrop &d) {
    if (d.removed || !d.labelled(held))
      return;
    SDL_FPoint g = gui(d.x, d.y - d.z);
    float gx = g.x, gy = g.y - 12 * u;
    if (gx < xo || gy < yo || gx > xo + s.vw * k || gy > yo + s.vh * k)
      return;
    std::string text = d.label();
    float w = Screen::textWidth(text, small) + 6 * u, h = 16 * small + 2 * u;
    labels.push_back({gx - w / 2, gy - h, w, h, std::move(text), d.color()});
  });
  std::sort(labels.begin(), labels.end(), [](const Tag &a, const Tag &b) { return a.y > b.y; });
  for (size_t i = 0; i < labels.size(); i++) {
    Tag &t = labels[i];
    for (int tries = 0; tries < 12; tries++) {
      const Tag *hit = nullptr;
      for (size_t j = 0; j < i && !hit; j++) {
        const Tag &o = labels[j];
        if (t.x < o.x + o.w && o.x < t.x + t.w && t.y < o.y + o.h && o.y < t.y + t.h)
          hit = &o;
      }
      if (!hit)
        break;
      t.y = hit->y - t.h - u;
    }
    s.rect(t.x, t.y, t.w, t.h, col::panel.alpha(0.82f));
    s.text(t.text, t.x + 3 * u, t.y + u, small, t.c);
  }
}
