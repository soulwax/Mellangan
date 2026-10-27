#include <spdlog/fmt/fmt.h>

#include <array>
#include <string>
#include <vector>

#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {
struct View {
    float xo, yo;
    double xw, yw;
    float k;

    float guiX(double x) const { return float(xo + (x - xw) * k); }
    float guiY(double y) const { return float(yo + (y - yw) * k); }
    double worldX(float x) const { return xw + (x - xo) / k; }
    double worldY(float y) const { return yw + (y - yo) / k; }
    bool inside(float x, float y) const { return x >= xo && y >= yo && x < xo + Screen::W * k && y < yo + Screen::H * k; }
};

void dashed(Screen &s, float x0, float y0, float x1, float y1, float width, float dash, float phase, Color c) {
    float xd = x1 - x0, yd = y1 - y0, len = std::sqrt(xd * xd + yd * yd);
    if (len < 1e-5f) return;
    xd /= len, yd /= len;
    for (float t = std::fmod(phase, dash * 2) - dash * 2; t < len; t += dash * 2) {
        float t0 = std::max(t, 0.0f), t1 = std::min(t + dash, len);
        if (t1 > t0) s.line(x0 + xd * t0, y0 + yd * t0, x0 + xd * t1, y0 + yd * t1, width, c);
    }
}
void brackets(Screen &s, float x, float y, float w, float h, float arm, float t, Color c) {
    s.rect(x, y, arm, t, c), s.rect(x, y, t, arm, c);
    s.rect(x + w - arm, y, arm, t, c), s.rect(x + w - t, y, t, arm, c);
    s.rect(x, y + h - t, arm, t, c), s.rect(x, y + h - arm, t, arm, c);
    s.rect(x + w - arm, y + h - t, arm, t, c), s.rect(x + w - t, y + h - arm, t, arm, c);
}

void panel(Screen &s, const std::vector<std::pair<std::string, Color>> &lines, float x, float y, float size, bool alignRight) {
    float wide = 0, lh = 17 * size, pad = 4 * size;
    for (auto &[text, c] : lines) wide = std::max(wide, Screen::textWidth(text, size));
    if (alignRight) x -= wide + pad * 2;
    s.rect(x, y, wide + pad * 2, lines.size() * lh + pad * 2, col::panel.alpha(0.82f));
    for (size_t i = 0; i < lines.size(); i++) s.text(lines[i].first, x + pad, y + pad + i * lh, size, lines[i].second);
}

} // namespace

void Level::renderWorldDebug(Screen &s, int xView, int yView, float xMouse, float yMouse) {
    float k = s.scale, u = s.u, small = std::max(1.0f, u - 1), t = SDL_GetTicks() / 1000.0f;
    View v{std::floor((s.w - Screen::W * k) / 2), std::floor((s.h - Screen::H * k) / 2), double(xView), double(yView), k};
    constexpr float RP = world::REGION * 16;

    int tx0 = int(std::floor(xView / 16.0)), ty0 = int(std::floor(yView / 16.0));
    int rx0 = world::regionOf(tx0), ry0 = world::regionOf(ty0);
    int rx1 = world::regionOf(tx0 + Screen::W / 16 + 1), ry1 = world::regionOf(ty0 + Screen::H / 16 + 1);
    Color grid{1, 1, 1, 0.16f};
    for (int rx = rx0; rx <= rx1 + 1; rx++) {
        float gx = v.guiX(rx * RP);
        if (gx >= v.xo && gx < v.xo + Screen::W * k) s.rect(gx, v.yo, 1, Screen::H * k, grid);
    }
    for (int ry = ry0; ry <= ry1 + 1; ry++) {
        float gy = v.guiY(ry * RP);
        if (gy >= v.yo && gy < v.yo + Screen::H * k) s.rect(v.xo, gy, Screen::W * k, 1, grid);
    }
    for (int ry = ry0; ry <= ry1; ry++)
        for (int rx = rx0; rx <= rx1; rx++) {
            float gx = v.guiX(rx * RP) + 3 * small, gy = v.guiY(ry * RP) + 3 * small;
            if (v.inside(gx, gy)) s.text(fmt::format("R {},{}", rx, ry), gx, gy, small, grid.alpha(0.45f));
        }
    int hx = int(std::floor(v.worldX(xMouse) / 16)), hy = int(std::floor(v.worldY(yMouse) / 16));
    bool hovering = v.inside(xMouse, yMouse);
    Structure hovered{};
    bool hoverStructure = hovering && world::structureAt(hx, hy, hovered, 1);

    Structure target{};
    bool targeted = world::nearestStructure(player->x, player->y, 8, target, Level::unvisited, this);

    auto box = [&](const Structure &st, int margin) {
        return std::array<float, 4>{v.guiX((st.x - margin) * 16.0), v.guiY((st.y - margin) * 16.0), (st.w + margin * 2) * 16 * k, (st.h + margin * 2) * 16 * k};
    };

    int seen = 0;
    Structure st;
    for (int ry = ry0 - 1; ry <= ry1 + 1; ry++)
        for (int rx = rx0 - 1; rx <= rx1 + 1; rx++) {
            if (!world::structureIn(rx, ry, st)) continue;
            auto [x, y, w, h] = box(st, 0);
            if (x + w < v.xo || y + h < v.yo || x > v.xo + Screen::W * k || y > v.yo + Screen::H * k)
                continue;
            seen++;
            bool hot = hoverStructure && hovered.rx == rx && hovered.ry == ry;
            Color c = st.debugColor();
            auto [ax, ay, aw, ah] = box(st, 1);
            s.frame(ax, ay, aw, ah, 1, c.alpha(0.3f));
            s.rect(x, y, w, h, c.alpha(hot ? 0.22f : 0.1f));
            s.frame(x, y, w, h, hot ? 2 * u : u, hot ? col::white : c.alpha(0.9f));
            std::string label = fmt::format("{} {}x{}", st.name(), st.w, st.h);
            s.text(label, x, ay - 17 * small - u, small, c);
        }

    if (targeted) {
        Color c = target.debugColor();
        auto [x, y, w, h] = box(target, 1);
        float pulse = (2 + 2 * std::sin(t * 6)) * u;
        brackets(s, x - pulse, y - pulse, w + pulse * 2, h + pulse * 2, 6 * u, u, c);
        float xf = v.guiX(player->x), yf = v.guiY(player->bodyY());
        float xt = v.guiX(target.centerX()), yt = v.guiY(target.centerY());
        float xm = v.guiX(xView + Screen::W * 0.5), ym = v.guiY(yView + Screen::H * 0.5);
        float xh = Screen::W * k * 0.5f - 14 * u, yh = Screen::H * k * 0.5f - 14 * u, xd = xt - xm, yd = yt - ym;
        float fit = std::min(xd != 0 ? xh / std::abs(xd) : INFINITY, yd != 0 ? yh / std::abs(yd) : INFINITY);
        bool offscreen = fit < 1;
        float xe = offscreen ? xm + xd * fit : xt, ye = offscreen ? ym + yd * fit : yt;
        dashed(s, xf, yf, xe, ye, u, 4 * u, t * 24 * u, c.alpha(0.75f));

        double xs = target.centerX() - player->x, ys = target.centerY() - player->y;
        float tiles = float(std::sqrt(xs * xs + ys * ys) / 16);
        std::string tag = fmt::format("{} {:.0f}t", target.name(), tiles);
        if (offscreen) {
            float dd = std::sqrt(xd * xd + yd * yd), xn = xd / dd, yn = yd / dd;
            s.line(xe, ye, xe - xn * 8 * u - yn * 5 * u, ye - yn * 8 * u + xn * 5 * u, 2 * u, c);
            s.line(xe, ye, xe - xn * 8 * u + yn * 5 * u, ye - yn * 8 * u - xn * 5 * u, 2 * u, c);
            s.textCenter(tag, xe - xn * 18 * u, ye - yn * 18 * u - 8 * small, small, c);
        } else {
            s.textCenter(tag, xt, y + h + pulse + 2 * u, small, c);
        }
    }

    if (hovering) {
        s.frame(v.guiX(hx * 16.0), v.guiY(hy * 16.0), 16 * k, 16 * k, u, col::white.alpha(0.8f));
        Climate hc = world::climateAt(hx, hy);
        const BiomeDef *hb = &BIOMES[world::biomeOf(hc)];
        std::vector<std::pair<std::string, Color>> info{
            {fmt::format("tile {},{}  region {},{}", hx, hy, world::regionOf(hx), world::regionOf(hy)), col::white},
            {fmt::format("{}  clump {:.2f}  {}", hb->name, world::clumpAt(hx, hy), world::isDirt(hx, hy) ? "dirt" : "grass"),
             hb->debug},
            {fmt::format("temp {:.2f}  moist {:.2f}  eerie {:.2f}", hc.temp, hc.moist, hc.eerie), col::muted},
        };
        Prop p;
        bool prop = world::propAt(hx, hy, p);
        if (hoverStructure) {
            bool inside = hovered.contains(hx, hy);
            info.push_back({fmt::format("{} {} {},{}", inside ? "in" : "apron of", hovered.name(), hx - hovered.x, hy - hovered.y),
                            hovered.debugColor()});
        }
        info.push_back({prop ? fmt::format("prop: {} tile{}{}", p.count, p.count > 1 ? "s" : "", p.light.a > 0 ? ", lit" : "")
                             : "no prop",
                        col::muted});
        float px = xMouse + 12 * u, py = yMouse + 12 * u;
        if (px + 200 * small > s.w) px = xMouse - 200 * small;
        panel(s, info, px, py, small, false);
    }

    const BiomeDef *pb = &BIOMES[world::biomeAt(int(std::floor(player->x / 16)), int(std::floor(player->y / 16)))];
    std::vector<std::pair<std::string, Color>> hud{
        {"WORLD DEBUG  F8 hide  F9 warp", col::gold},
        {fmt::format("player tile {},{}  region {},{}", int(std::floor(player->x / 16)), int(std::floor(player->y / 16)),
                     world::regionOf(int(std::floor(player->x / 16))), world::regionOf(int(std::floor(player->y / 16)))),
         col::white},
        {fmt::format("biome {}", pb->name), pb->debug},
        {fmt::format("{} structures in view", seen), col::muted},
        {targeted ? fmt::format("target {} at {},{}", target.name(), target.x, target.y) : "no structure within 8 regions",
         targeted ? target.debugColor() : col::muted},
    };
    for (int i = 0; i < int(StructureKind::Count); i++) {
        Structure legend{StructureKind(i)};
        hud.push_back({fmt::format("# {}", legend.name()), legend.debugColor()});
    }
    panel(s, hud, s.w - 4 * u, 60 * u, small, true);
}
