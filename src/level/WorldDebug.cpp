#include "level/WorldDebug.hpp"

#include <spdlog/fmt/fmt.h>

#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {

constexpr float RP = world::REGION * 16;

void dashed(Screen &s, float x0, float y0, float x1, float y1, float width,
            float dash, float phase, Color c) {
  float xd = x1 - x0, yd = y1 - y0, len = std::sqrt(xd * xd + yd * yd);
  if (len < 1e-5f)
    return;
  xd /= len, yd /= len;
  for (float t = std::fmod(phase, dash * 2) - dash * 2; t < len;
       t += dash * 2) {
    float t0 = std::max(t, 0.0f), t1 = std::min(t + dash, len);
    if (t1 > t0)
      s.line(x0 + xd * t0, y0 + yd * t0, x0 + xd * t1, y0 + yd * t1, width, c);
  }
}

void brackets(Screen &s, float x, float y, float w, float h, float arm, float t,
              Color c) {
  s.rect(x, y, arm, t, c), s.rect(x, y, t, arm, c);
  s.rect(x + w - arm, y, arm, t, c), s.rect(x + w - t, y, t, arm, c);
  s.rect(x, y + h - t, arm, t, c), s.rect(x, y + h - arm, t, arm, c);
  s.rect(x + w - arm, y + h - t, arm, t, c),
      s.rect(x + w - t, y + h - arm, t, arm, c);
}

} // namespace

WorldDebug::WorldDebug(Screen &s, Level &level, int xView, int yView)
    : s(s), level(level), xView(xView), yView(yView) {
  k = s.scale, u = s.u, small = std::max(1.0f, u - 1), t = SDL_GetTicks() / 1000.0f;
  xo = std::floor((s.w - s.vw * k) / 2), yo = std::floor((s.h - s.vh * k) / 2);
  int tx0 = int(std::floor(xView / 16.0)), ty0 = int(std::floor(yView / 16.0));
  rx0 = world::regionOf(tx0), ry0 = world::regionOf(ty0);
  rx1 = world::regionOf(tx0 + s.vw / 16 + 1), ry1 = world::regionOf(ty0 + s.vh / 16 + 1);
}

bool WorldDebug::inside(float x, float y) const {
  return x >= xo && y >= yo && x < xo + s.vw * k && y < yo + s.vh * k;
}

std::array<float, 4> WorldDebug::box(const Structure &st, int margin) const {
  return {guiX((st.x - margin) * 16.0), guiY((st.y - margin) * 16.0),
          (st.w + margin * 2) * 16 * k, (st.h + margin * 2) * 16 * k};
}

void WorldDebug::panel(const Lines &lines, float x, float y, bool alignRight) {
  float wide = 0, lh = 17 * small, pad = 4 * small;
  for (auto &[text, c] : lines)
    wide = std::max(wide, Screen::textWidth(text, small));
  if (alignRight)
    x -= wide + pad * 2;
  s.rect(x, y, wide + pad * 2, lines.size() * lh + pad * 2, col::panel.alpha(0.82f));
  for (size_t i = 0; i < lines.size(); i++)
    s.text(lines[i].first, x + pad, y + pad + i * lh, small, lines[i].second);
}

void WorldDebug::regions() {
  Color grid{1, 1, 1, 0.16f};
  for (int rx = rx0; rx <= rx1 + 1; rx++) {
    float gx = guiX(rx * RP);
    if (gx >= xo && gx < xo + s.vw * k)
      s.rect(gx, yo, 1, s.vh * k, grid);
  }
  for (int ry = ry0; ry <= ry1 + 1; ry++) {
    float gy = guiY(ry * RP);
    if (gy >= yo && gy < yo + s.vh * k)
      s.rect(xo, gy, s.vw * k, 1, grid);
  }
  for (int ry = ry0; ry <= ry1; ry++)
    for (int rx = rx0; rx <= rx1; rx++) {
      float gx = guiX(rx * RP) + 3 * small, gy = guiY(ry * RP) + 3 * small;
      if (inside(gx, gy))
        s.text(fmt::format("R {},{}", rx, ry), gx, gy, small, grid.alpha(0.45f));
    }
}

int WorldDebug::structures(const Structure *hovered) {
  int seen = 0;
  Structure st;
  for (int ry = ry0 - 1; ry <= ry1 + 1; ry++)
    for (int rx = rx0 - 1; rx <= rx1 + 1; rx++) {
      if (!world::structureIn(rx, ry, st))
        continue;
      auto [x, y, w, h] = box(st, 0);
      if (x + w < xo || y + h < yo || x > xo + s.vw * k || y > yo + s.vh * k)
        continue;
      seen++;
      bool hot = hovered && hovered->rx == rx && hovered->ry == ry;
      Color c = st.debugColor();
      auto [ax, ay, aw, ah] = box(st, 1);
      s.frame(ax, ay, aw, ah, 1, c.alpha(0.3f));
      s.rect(x, y, w, h, c.alpha(hot ? 0.22f : 0.1f));
      s.frame(x, y, w, h, hot ? 2 * u : u, hot ? col::white : c.alpha(0.9f));
      s.text(fmt::format("{} {}x{}", st.name(), st.w, st.h), x, ay - 17 * small - u,
             small, c);
    }
  return seen;
}

void WorldDebug::target(const Structure &target) {
  Player &p = *level.player;
  Color c = target.debugColor();
  auto [x, y, w, h] = box(target, 1);
  float pulse = (2 + 2 * std::sin(t * 6)) * u;
  brackets(s, x - pulse, y - pulse, w + pulse * 2, h + pulse * 2, 6 * u, u, c);
  float xf = guiX(p.x), yf = guiY(p.bodyY());
  float xt = guiX(target.centerX()), yt = guiY(target.centerY());
  float xm = guiX(xView + s.vw * 0.5), ym = guiY(yView + s.vh * 0.5);
  float xh = s.vw * k * 0.5f - 14 * u, yh = s.vh * k * 0.5f - 14 * u, xd = xt - xm,
        yd = yt - ym;
  float fit = std::min(xd != 0 ? xh / std::abs(xd) : INFINITY,
                       yd != 0 ? yh / std::abs(yd) : INFINITY);
  bool offscreen = fit < 1;
  float xe = offscreen ? xm + xd * fit : xt, ye = offscreen ? ym + yd * fit : yt;
  dashed(s, xf, yf, xe, ye, u, 4 * u, t * 24 * u, c.alpha(0.75f));

  double xs = target.centerX() - p.x, ys = target.centerY() - p.y;
  float tiles = float(std::sqrt(xs * xs + ys * ys) / 16);
  std::string tag = fmt::format("{} {:.0f}t", target.name(), tiles);
  if (!offscreen)
    return s.textCenter(tag, xt, y + h + pulse + 2 * u, small, c);
  float dd = std::sqrt(xd * xd + yd * yd), xn = xd / dd, yn = yd / dd;
  s.line(xe, ye, xe - xn * 8 * u - yn * 5 * u, ye - yn * 8 * u + xn * 5 * u, 2 * u, c);
  s.line(xe, ye, xe - xn * 8 * u + yn * 5 * u, ye - yn * 8 * u - xn * 5 * u, 2 * u, c);
  s.textCenter(tag, xe - xn * 18 * u, ye - yn * 18 * u - 8 * small, small, c);
}

void WorldDebug::hover(int hx, int hy, float xMouse, float yMouse,
                       const Structure *hovered) {
  s.frame(guiX(hx * 16.0), guiY(hy * 16.0), 16 * k, 16 * k, u, col::white.alpha(0.8f));
  Climate hc = world::climateAt(hx, hy);
  const Biome *hb = BIOMES[world::biomeOf(hc)];
  Lines info{
      {fmt::format("tile {},{}  region {},{}", hx, hy, world::regionOf(hx),
                   world::regionOf(hy)),
       col::white},
      {fmt::format("{}  clump {:.2f}  {}", hb->name, world::clumpAt(hx, hy),
                   world::isDirt(hx, hy) ? "dirt" : "grass"),
       hb->debug},
      {fmt::format("temp {:.2f}  moist {:.2f}  eerie {:.2f}", hc.temp, hc.moist,
                   hc.eerie),
       col::muted},
  };
  Prop p;
  bool prop = world::propAt(hx, hy, p);
  if (hovered)
    info.push_back({fmt::format("{} {} {},{}", hovered->contains(hx, hy) ? "in" : "apron of",
                                hovered->name(), hx - hovered->x, hy - hovered->y),
                    hovered->debugColor()});
  info.push_back({prop ? fmt::format("prop: {} tile{}{}", p.count, p.count > 1 ? "s" : "",
                                     p.light.a > 0 ? ", lit" : "")
                       : "no prop",
                  col::muted});
  float px = xMouse + 12 * u, py = yMouse + 12 * u;
  if (px + 200 * small > s.w)
    px = xMouse - 200 * small;
  panel(info, px, py, false);
}

void WorldDebug::legend(int seen, const Structure *target) {
  Player &p = *level.player;
  int tx = world::tileOf(p.x), ty = world::tileOf(p.y);
  const Biome *pb = BIOMES[world::biomeAt(tx, ty)];
  Lines hud{
      {"WORLD DEBUG  F8 hide  F9 warp", col::gold},
      {fmt::format("player tile {},{}  region {},{}", tx, ty, world::regionOf(tx),
                   world::regionOf(ty)),
       col::white},
      {fmt::format("biome {}", pb->name), pb->debug},
      {fmt::format("{} structures in view", seen), col::muted},
      {target ? fmt::format("target {} at {},{}", target->name(), target->x, target->y)
              : "no structure within 8 regions",
       target ? target->debugColor() : col::muted},
  };
  for (int i = 0; i < int(StructureKind::Count); i++) {
    Structure kind{};
    kind.kind = StructureKind(i);
    hud.push_back({fmt::format("# {}", kind.name()), kind.debugColor()});
  }
  panel(hud, s.w - 4 * u, 60 * u, true);
}

void WorldDebug::render(float xMouse, float yMouse) {
  regions();
  int hx = int(std::floor(worldX(xMouse) / 16)), hy = int(std::floor(worldY(yMouse) / 16));
  bool hovering = inside(xMouse, yMouse);
  Structure hovered{}, goal{};
  bool hoverStructure = hovering && world::structureAt(hx, hy, hovered, 1);
  bool targeted = world::nearestStructure(level.player->x, level.player->y, 8, goal,
                                          Level::unvisited, &level);
  int seen = structures(hoverStructure ? &hovered : nullptr);
  if (targeted)
    target(goal);
  if (hovering)
    hover(hx, hy, xMouse, yMouse, hoverStructure ? &hovered : nullptr);
  legend(seen, targeted ? &goal : nullptr);
}
