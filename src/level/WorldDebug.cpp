#include <spdlog/fmt/fmt.h>

#include <array>
#include <string>
#include <vector>

#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {
struct View {
    Vec2 origin, world;
    float k;

    Vec2 gui(Vec2 p) const { return origin + (p - world) * k; }
    Vec2 toWorld(Vec2 g) const { return world + (g - origin) / k; }
    bool inside(Vec2 g) const {
        return g.x >= origin.x && g.y >= origin.y && g.x < origin.x + Screen::W * k && g.y < origin.y + Screen::H * k;
    }
};

void dashed(Screen &s, Vec2 a, Vec2 b, float width, float dash, float phase, Color c) {
    float len = length(b - a);
    Vec2 d = normalized(b - a);
    for (float t = std::fmod(phase, dash * 2) - dash * 2; t < len; t += dash * 2) {
        float t0 = std::max(t, 0.0f), t1 = std::min(t + dash, len);
        if (t1 > t0) s.line(a + d * t0, a + d * t1, width, c);
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

void Level::renderWorldDebug(Screen &s, Vec2 viewOffset, Vec2 mouse) {
    float k = s.scale, u = s.u, small = std::max(1.0f, u - 1), t = SDL_GetTicks() / 1000.0f;
    View v{{std::floor((s.w - Screen::W * k) / 2), std::floor((s.h - Screen::H * k) / 2)}, viewOffset, k};
    constexpr float RP = world::REGION * 16;

    int tx0 = int(std::floor(viewOffset.x / 16)), ty0 = int(std::floor(viewOffset.y / 16));
    int rx0 = world::regionOf(tx0), ry0 = world::regionOf(ty0);
    int rx1 = world::regionOf(tx0 + Screen::W / 16 + 1), ry1 = world::regionOf(ty0 + Screen::H / 16 + 1);
    Color grid{1, 1, 1, 0.16f};
    for (int rx = rx0; rx <= rx1 + 1; rx++) {
        float gx = v.gui({rx * RP, 0}).x;
        if (gx >= v.origin.x && gx < v.origin.x + Screen::W * k) s.rect(gx, v.origin.y, 1, Screen::H * k, grid);
    }
    for (int ry = ry0; ry <= ry1 + 1; ry++) {
        float gy = v.gui({0, ry * RP}).y;
        if (gy >= v.origin.y && gy < v.origin.y + Screen::H * k) s.rect(v.origin.x, gy, Screen::W * k, 1, grid);
    }
    for (int ry = ry0; ry <= ry1; ry++)
        for (int rx = rx0; rx <= rx1; rx++) {
            Vec2 g = v.gui({rx * RP, ry * RP}) + Vec2{3, 3} * small;
            if (v.inside(g)) s.text(fmt::format("R {},{}", rx, ry), g.x, g.y, small, grid.alpha(0.45f));
        }
    Vec2 hw = v.toWorld(mouse);
    int hx = int(std::floor(hw.x / 16)), hy = int(std::floor(hw.y / 16));
    bool hovering = v.inside(mouse);
    Structure hovered{};
    bool hoverStructure = hovering && world::structureAt(hx, hy, hovered, 1);

    Structure target{};
    bool targeted = world::nearestStructure(player->pos, 8, target);

    auto box = [&](const Structure &st, int margin) {
        Vec2 a = v.gui({float(st.x - margin) * 16, float(st.y - margin) * 16});
        return std::array<float, 4>{a.x, a.y, (st.w + margin * 2) * 16 * k, (st.h + margin * 2) * 16 * k};
    };

    int seen = 0;
    Structure st;
    for (int ry = ry0 - 1; ry <= ry1 + 1; ry++)
        for (int rx = rx0 - 1; rx <= rx1 + 1; rx++) {
            if (!world::structureIn(rx, ry, st)) continue;
            auto [x, y, w, h] = box(st, 0);
            if (x + w < v.origin.x || y + h < v.origin.y || x > v.origin.x + Screen::W * k || y > v.origin.y + Screen::H * k)
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
        Vec2 from = v.gui(player->body()), to = v.gui(target.center()), mid = v.gui(viewOffset + Vec2{Screen::W, Screen::H} * 0.5f);
        Vec2 half = Vec2{Screen::W, Screen::H} * (k * 0.5f) - Vec2{14, 14} * u, d = to - mid;
        float fit = std::min(d.x != 0 ? half.x / std::abs(d.x) : INFINITY, d.y != 0 ? half.y / std::abs(d.y) : INFINITY);
        bool offscreen = fit < 1;
        Vec2 end = offscreen ? mid + d * fit : to;
        dashed(s, from, end, u, 4 * u, t * 24 * u, c.alpha(0.75f));

        float tiles = length(target.center() - player->pos) / 16;
        std::string tag = fmt::format("{} {:.0f}t", target.name(), tiles);
        if (offscreen) {
            Vec2 dir = normalized(d), side = perp(dir);
            s.line(end, end - dir * 8 * u + side * 5 * u, 2 * u, c);
            s.line(end, end - dir * 8 * u - side * 5 * u, 2 * u, c);
            Vec2 at = end - dir * 18 * u;
            s.textCenter(tag, at.x, at.y - 8 * small, small, c);
        } else {
            s.textCenter(tag, to.x, y + h + pulse + 2 * u, small, c);
        }
    }

    if (hovering) {
        Vec2 g = v.gui({hx * 16.0f, hy * 16.0f});
        s.frame(g.x, g.y, 16 * k, 16 * k, u, col::white.alpha(0.8f));
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
        float px = mouse.x + 12 * u, py = mouse.y + 12 * u;
        if (px + 200 * small > s.w) px = mouse.x - 200 * small;
        panel(s, info, px, py, small, false);
    }

    const BiomeDef *pb = &BIOMES[world::biomeAt(int(std::floor(player->pos.x / 16)), int(std::floor(player->pos.y / 16)))];
    std::vector<std::pair<std::string, Color>> hud{
        {"WORLD DEBUG  F8 hide  F9 warp", col::gold},
        {fmt::format("player tile {},{}  region {},{}", int(std::floor(player->pos.x / 16)), int(std::floor(player->pos.y / 16)),
                     world::regionOf(int(std::floor(player->pos.x / 16))), world::regionOf(int(std::floor(player->pos.y / 16)))),
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
