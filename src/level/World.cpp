#include "level/World.hpp"

namespace world {

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;

float valueNoise(float x, float y) {
    float xf = std::floor(x), yf = std::floor(y), tx = x - xf, ty = y - yf;
    int xi = int(xf), yi = int(yf);
    auto h = [](int a, int b) { return float(hash2(a, b) & 0xFFFF) / 65535.0f; };
    tx = tx * tx * (3 - 2 * tx), ty = ty * ty * (3 - 2 * ty);
    return lerpf(lerpf(h(xi, yi), h(xi + 1, yi), tx), lerpf(h(xi, yi + 1), h(xi + 1, yi + 1), tx), ty);
}

bool autumnAt(int x, int y) { return valueNoise(x * 0.04f + 300, y * 0.04f - 40) > 0.62f; }

uint32_t roll(const Structure &s, int lx, int ly) { return hash2(int(s.seed) ^ (lx * 92821), ly * 68917 + 5); }

bool structureDirt(const Structure &s, int lx, int ly) {
    if (lx < 0 || ly < 0 || lx >= s.w || ly >= s.h) return false;
    if (s.kind == StructureKind::Ruin) return true;
    if (s.kind != StructureKind::Camp) return false;
    float dx = lx - (s.w - 1) * 0.5f, dy = ly - (s.h - 1) * 0.5f;
    return dx * dx + dy * dy < 2.7f * 2.7f;
}

bool cottage(const Structure &s, int lx, int ly, uint32_t r, Prop &p) {
    bool red = s.seed & 1, stone = s.seed & 2, lit = (s.seed >> 5) % 3 != 0;
    if (ly == 2 && lx >= 1 && lx <= 3) {
        int c = lx - 1, roof = red ? tt::RedRoof : tt::GreyRoof;
        bool door = c == 1;
        p.add(atlas::town(door ? (stone ? tt::StoneDoor : tt::WoodDoor) : (stone ? tt::StoneWindow : tt::WoodWindow)));
        p.add(atlas::town(roof + 12 + c), {0, -16});
        p.add(atlas::town(door && (s.seed & 4) ? roof + 3 : roof + c), {0, -32});
        p.shadow = 1.4f;
        if (!door && lit) p.light = {0.34f, 0.2f, 0.06f, 1}, p.lightOffset = {0, -8}, p.lightSize = 56, p.wisp = false;
        return true;
    }
    if (lx == 0 && ly == 3) return p.add(atlas::tile(r & 1 ? t::Barrel : t::Crate)), p.shadow = 0.8f, true;
    if (lx == 4 && ly == 3) {
        if (s.seed & 8) return p.add(atlas::town(tt::Beehive)), p.shadow = 0.8f, true;
        p.add(atlas::town(tt::Well)), p.add(atlas::town(tt::WellRoof), {0, -16}), p.shadow = 1.0f;
        return true;
    }
    if (lx == 1 && ly == 4 && (s.seed & 16)) return p.add(atlas::town(tt::Sign)), p.shadow = 0.6f, true;
    return false;
}
bool graveyard(const Structure &s, int lx, int ly, uint32_t r, Prop &p) {
    int right = s.w - 1, bottom = s.h - 1, gate = s.w / 2;
    if (ly == 0) return p.add(atlas::town(lx == 0 ? tt::FenceTopLeft : lx == right ? tt::FenceTopRight : tt::FenceTop)), true;
    if (ly == bottom) {
        if (lx == gate) return false;
        int f = lx == 0 ? tt::FenceBottomLeft : lx == right ? tt::FenceBottomRight
              : lx == gate - 1 ? tt::FenceEndRight : lx == gate + 1 ? tt::FenceEndLeft : tt::FenceRail;
        return p.add(atlas::town(f)), true;
    }
    if (lx == 0 || lx == right) return p.add(atlas::town(lx == 0 ? tt::FenceLeft : tt::FenceRight)), true;
    if (lx % 2 == 1 && ly % 2 == 0 && r % 5 != 0) {
        p.add(atlas::tile(r % 3 == 0 ? t::GraveCross : t::GraveStone));
        if (r % 4 == 0) p.light = {0.07f, 0.15f, 0.32f, 1};
        return true;
    }
    return false;
}
bool shrine(const Structure &s, int lx, int ly, uint32_t r, Prop &p) {
    bool corner = (lx == 0 || lx == s.w - 1) && (ly == 0 || ly == s.h - 1);
    if (corner) {
        p.add(atlas::tile(t::Pillar));
        if (r % 3 != 0) p.add(atlas::tile(t::Pillar), {0, -16});
        p.shadow = 1.0f;
        return true;
    }
    if (lx == s.w / 2 && ly == s.h / 2) {
        p.add(atlas::tile(t::Crystal)), p.shadow = 0.7f;
        p.light = {0.06f, 0.22f, 0.38f, 1}, p.lightSize = 130, p.lightOffset = {0, -12};
        return true;
    }
    return false;
}
bool camp(const Structure &s, int lx, int ly, uint32_t r, Prop &p) {
    int c = s.w / 2;
    if (lx == c && ly == c) {
        p.add(atlas::town(tt::Log));
        p.light = {0.55f, 0.27f, 0.07f, 1}, p.lightSize = 150, p.lightOffset = {0, -6}, p.wisp = false;
        return true;
    }
    if ((lx == c - 2 || lx == c + 2) && ly == c) return p.add(atlas::town(tt::Log)), p.shadow = 0.9f, true;
    if (lx == c && ly == c + 2 && (s.seed & 1)) return p.add(atlas::town(tt::Log)), p.shadow = 0.9f, true;
    if ((lx == 1 && ly == 1) || (lx == 1 && ly == 2 && (r & 1))) return p.add(atlas::town(tt::Sack)), p.shadow = 0.6f, true;
    if ((lx == s.w - 2 && ly == 1) || (lx == s.w - 1 && ly == 2 && (r & 1)))
        return p.add(atlas::tile(r & 2 ? t::Crate : t::Barrel)), p.shadow = 0.8f, true;
    return false;
}
bool ruin(const Structure &s, int lx, int ly, uint32_t r, Prop &p) {
    int arch = 1 + int(s.seed % uint32_t(s.w - 3));
    if (ly == 1) {
        if (lx == arch || lx == arch + 1) {
            p.add(atlas::town(lx == arch ? tt::ArchLeft : tt::ArchRight));
            p.add(atlas::town(tt::Bricks), {0, -16});
        } else if (r % 10 < 7) {
            p.add(atlas::town(tt::Bricks));
            if (r % 10 < 3) p.add(atlas::town(tt::Bricks), {0, -16});
        } else {
            return false;
        }
        p.shadow = 1.2f;
        return true;
    }
    if (ly < 2) return false;
    if (r % 11 == 0) return p.add(atlas::town(tt::Bricks)), p.shadow = 1.0f, true;
    if (r % 13 == 0) return p.add(atlas::town(tt::Mushrooms)), p.light = {0.03f, 0.12f, 0.10f, 1}, true;
    return false;
}

bool structureProp(const Structure &s, int lx, int ly, Prop &p) {
    uint32_t r = roll(s, lx, ly);
    switch (s.kind) {
    case StructureKind::Cottage: return cottage(s, lx, ly, r, p);
    case StructureKind::Graveyard: return graveyard(s, lx, ly, r, p);
    case StructureKind::Shrine: return shrine(s, lx, ly, r, p);
    case StructureKind::Camp: return camp(s, lx, ly, r, p);
    case StructureKind::Ruin: return ruin(s, lx, ly, r, p);
    default: return false;
    }
}

} // namespace

bool structureIn(int rx, int ry, Structure &s) {
    uint32_t h = hash2(rx * 31 + 7, ry * 17 - 3);
    if (h % 100 >= 55) return false;

    int cx = rx * REGION + REGION / 2, cy = ry * REGION + REGION / 2, k = (h >> 8) % 100;
    float forest = forestAt(cx, cy), yard = yardAt(cx, cy);
    using K = StructureKind;
    K kind = yard > 0.6f     ? (k < 75 ? K::Graveyard : K::Shrine)
             : forest > 0.6f ? (k < 55 ? K::Camp : K::Ruin)
             : k < 40        ? K::Cottage
             : k < 60        ? K::Shrine
             : k < 75        ? K::Ruin
             : k < 88        ? K::Graveyard
                             : K::Camp;

    int w = 5, ht = 5;
    if (kind == K::Graveyard) w = 7 + 2 * ((h >> 15) & 1), ht = 6 + ((h >> 16) & 1);
    if (kind == K::Camp) w = ht = 7;
    if (kind == K::Ruin) w = 6 + ((h >> 15) & 1), ht = 4;
    int spanX = REGION - 4 - w, spanY = REGION - 4 - ht;
    int x = rx * REGION + 2 + int((h >> 18) % uint32_t(spanX + 1)), y = ry * REGION + 2 + int((h >> 24) % uint32_t(spanY + 1));
    if (x - 1 < 6 && x + w + 1 > -6 && y - 1 < 6 && y + ht + 1 > -6) return false; // keep the spawn clear

    s = {kind, x, y, w, ht, rx, ry, hash2(int(h), rx ^ (ry << 8))};
    return true;
}

bool structureAt(int x, int y, Structure &out, int margin) {
    return structureIn(regionOf(x), regionOf(y), out) && out.contains(x, y, margin);
}

bool nearestStructure(Vec2 p, int maxRegions, Structure &out) {
    int tx = int(std::floor(p.x / 16)), ty = int(std::floor(p.y / 16)), rx = regionOf(tx), ry = regionOf(ty);
    float best = INFINITY;
    Structure s;
    for (int y = ry - maxRegions; y <= ry + maxRegions; y++)
        for (int x = rx - maxRegions; x <= rx + maxRegions; x++) {
            if (!structureIn(x, y, s) || s.contains(tx, ty)) continue;
            float d = lengthSq(s.center() - p);
            if (d < best) best = d, out = s;
        }
    return best < INFINITY;
}

float forestAt(int x, int y) { return valueNoise(x * 0.09f + 50, y * 0.09f); }
float yardAt(int x, int y) { return valueNoise(x * 0.05f - 90, y * 0.05f + 20); }

bool isDirt(int x, int y) {
    Structure s;
    if (structureAt(x, y, s, 1)) return structureDirt(s, x - s.x, y - s.y);
    return valueNoise(x * 0.11f - 200, y * 0.11f + 77) > 0.79f;
}
Sprite groundTile(int x, int y) {
    Structure st;
    if (structureAt(x, y, st)) {
        int lx = x - st.x, ly = y - st.y;
        if (st.kind == StructureKind::Shrine) {
            int row = ly == 0 ? 0 : ly == st.h - 1 ? 2 : 1, col = lx == 0 ? 0 : lx == st.w - 1 ? 2 : 1;
            return atlas::town(tt::PlazaTopLeft + row * 12 + col);
        }
        if (st.kind == StructureKind::Cottage && lx == 2 && ly >= 3) return atlas::town(tt::Path);
    }
    if (!isDirt(x, y)) {
        int r = hash2(x, y) % 100;
        return atlas::town(r < 2 ? tt::GrassFlowers : r < 20 ? tt::GrassTufts : tt::Grass);
    }
    bool n = isDirt(x, y - 1), s = isDirt(x, y + 1), w = isDirt(x - 1, y), e = isDirt(x + 1, y);
    int row = n == s ? 1 : n ? 2 : 0, col = w == e ? 1 : w ? 2 : 0;
    return atlas::town(tt::DirtTopLeft + row * 12 + col);
}

float groundShade(int x, int y) { return valueNoise(x * 0.13f, y * 0.13f); }

bool propAt(int x, int y, Prop &p) {
    p = Prop{};
    p.pos = {x * 16 + 8.0f, y * 16 + 16.0f};

    Structure st;
    if (structureAt(x, y, st, 1)) return st.contains(x, y) && structureProp(st, x - st.x, y - st.y, p);
    if ((std::abs(x) < 4 && std::abs(y) < 4) || isDirt(x, y)) return false;

    uint32_t h = hash2(x * 7 + 3, y * 13 + 1);
    int r = (h >> 4) % 1000, pick = (h >> 16) % 10;
    float forest = forestAt(x, y), yard = yardAt(x, y);
    bool dense = forest > 0.65f, autumn = autumnAt(x, y);
    int trees = 6 + int(clampf((forest - 0.55f) * 2000, 0, 750)), graves = yard > 0.65f ? 70 : 3;

    if (r < trees) {
        int tall = dense ? 3 : 5, bush = dense ? 10 : 8;
        if (pick < tall) {
            p.add(atlas::town(autumn ? tt::AutumnTrunk : tt::Trunk));
            p.add(atlas::town(autumn ? tt::AutumnCrown : tt::Crown), {0, -16});
            p.shadow = 1.3f;
        } else if (pick < bush) {
            p.add(atlas::town(autumn ? tt::AutumnPine : tt::Pine)), p.shadow = 1.0f;
        } else {
            p.add(atlas::town(tt::Bush)), p.shadow = 0.9f;
        }
        return true;
    }
    if ((r -= trees) < graves) {
        p.add(atlas::tile(pick < 3 ? t::GraveCross : t::GraveStone));
        if (pick % 3 == 0) p.light = {0.07f, 0.15f, 0.32f, 1};
        return true;
    }
    struct Odd {
        int tile;
        bool town;
        float shadow;
        Color light;
    };
    constexpr Color none{0, 0, 0, 0};
    static const Odd odds[] = {
        {tt::Mushrooms, true, 0, {0.03f, 0.12f, 0.10f, 1}}, {tt::Sprouts, true, 0, none},
        {tt::Log, true, 0.9f, none},                         {tt::Beehive, true, 0.8f, none},
        {tt::Sign, true, 0.6f, none},                        {tt::Sack, true, 0.6f, none},
        {t::Crystal, false, 0.7f, {0.05f, 0.18f, 0.30f, 1}}, {tt::Well, true, 1.0f, none},
    };
    r -= graves;
    if (r >= 2 * int(std::size(odds))) return false;
    const Odd &o = odds[r / 2];
    p.add(o.town ? atlas::town(o.tile) : atlas::tile(o.tile));
    if (o.tile == tt::Well && o.town) p.add(atlas::town(tt::WellRoof), {0, -16});
    p.shadow = o.shadow, p.light = o.light;
    return true;
}

} // namespace world

const char *Structure::name() const {
    static const char *names[] = {"COTTAGE", "GRAVEYARD", "SHRINE", "CAMP", "RUIN"};
    return names[int(kind)];
}

Color Structure::debugColor() const {
    static const Color colors[] = {
        {1.0f, 0.62f, 0.25f, 1}, {0.62f, 0.55f, 1.0f, 1}, {0.3f, 0.9f, 1.0f, 1}, {1.0f, 0.35f, 0.3f, 1}, {0.6f, 0.9f, 0.4f, 1},
    };
    return colors[int(kind)];
}
