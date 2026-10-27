#include "level/Structure.hpp"

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;

void sizeSquare(uint32_t, int *w, int *h) { *w = 5, *h = 5; }
void sizeGraveyard(uint32_t r, int *w, int *h) { *w = 7 + 2 * ((r >> 15) & 1), *h = 6 + ((r >> 16) & 1); }
void sizeCamp(uint32_t, int *w, int *h) { *w = 7, *h = 7; }
void sizeRuin(uint32_t r, int *w, int *h) { *w = 6 + ((r >> 15) & 1), *h = 4; }

bool campDirt(const Structure *s, int lx, int ly) {
    float dx = lx - (s->w - 1) * 0.5f, dy = ly - (s->h - 1) * 0.5f;
    return dx * dx + dy * dy < 2.7f * 2.7f;
}
bool ruinDirt(const Structure *, int, int) { return true; }

bool shrineGround(const Structure *s, int lx, int ly, Sprite *out) {
    int row = ly == 0 ? 0 : ly == s->h - 1 ? 2 : 1, col = lx == 0 ? 0 : lx == s->w - 1 ? 2 : 1;
    *out = atlas::town(tt::PlazaTopLeft + row * 12 + col);
    return true;
}
bool cottageGround(const Structure *, int lx, int ly, Sprite *out) {
    if (lx != 2 || ly < 3) return false;
    *out = atlas::town(tt::Path);
    return true;
}

bool cottage(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
    bool red = s->seed & 1, stone = s->seed & 2, lit = (s->seed >> 5) % 3 != 0;
    if (ly == 2 && lx >= 1 && lx <= 3) {
        int c = lx - 1, roof = red ? tt::RedRoof : tt::GreyRoof;
        bool door = c == 1;
        p->add(atlas::town(door ? (stone ? tt::StoneDoor : tt::WoodDoor) : (stone ? tt::StoneWindow : tt::WoodWindow)));
        p->add(atlas::town(roof + 12 + c), {0, -16});
        p->add(atlas::town(door && (s->seed & 4) ? roof + 3 : roof + c), {0, -32});
        p->shadow = 1.4f;
        if (!door && lit) p->light = {0.34f, 0.2f, 0.06f, 1}, p->lightOffset = {0, -8}, p->lightSize = 56, p->wisp = false;
        return true;
    }
    if (lx == 0 && ly == 3) return p->add(atlas::tile(r & 1 ? t::Barrel : t::Crate)), p->shadow = 0.8f, true;
    if (lx == 4 && ly == 3) {
        if (s->seed & 8) return p->add(atlas::town(tt::Beehive)), p->shadow = 0.8f, true;
        p->add(atlas::town(tt::Well)), p->add(atlas::town(tt::WellRoof), {0, -16}), p->shadow = 1.0f;
        return true;
    }
    if (lx == 1 && ly == 4 && (s->seed & 16)) return p->add(atlas::town(tt::Sign)), p->shadow = 0.6f, true;
    return false;
}
bool graveyard(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
    int right = s->w - 1, bottom = s->h - 1, gate = s->w / 2;
    if (ly == 0) return p->add(atlas::town(lx == 0 ? tt::FenceTopLeft : lx == right ? tt::FenceTopRight : tt::FenceTop)), true;
    if (ly == bottom) {
        if (lx == gate) return false;
        int f = lx == 0 ? tt::FenceBottomLeft : lx == right ? tt::FenceBottomRight
              : lx == gate - 1 ? tt::FenceEndRight : lx == gate + 1 ? tt::FenceEndLeft : tt::FenceRail;
        return p->add(atlas::town(f)), true;
    }
    if (lx == 0 || lx == right) return p->add(atlas::town(lx == 0 ? tt::FenceLeft : tt::FenceRight)), true;
    if (lx % 2 == 1 && ly % 2 == 0 && r % 5 != 0) {
        p->add(atlas::tile(r % 3 == 0 ? t::GraveCross : t::GraveStone));
        if (r % 4 == 0) p->light = {0.07f, 0.15f, 0.32f, 1};
        return true;
    }
    return false;
}
bool shrine(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
    bool corner = (lx == 0 || lx == s->w - 1) && (ly == 0 || ly == s->h - 1);
    if (corner) {
        p->add(atlas::tile(t::Pillar));
        if (r % 3 != 0) p->add(atlas::tile(t::Pillar), {0, -16});
        p->shadow = 1.0f;
        return true;
    }
    if (lx == s->w / 2 && ly == s->h / 2) {
        p->add(atlas::tile(t::Crystal)), p->shadow = 0.7f;
        p->light = {0.06f, 0.22f, 0.38f, 1}, p->lightSize = 130, p->lightOffset = {0, -12};
        return true;
    }
    return false;
}
bool camp(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
    int c = s->w / 2;
    if (lx == c && ly == c) {
        p->add(atlas::town(tt::Log));
        p->light = {0.55f, 0.27f, 0.07f, 1}, p->lightSize = 150, p->lightOffset = {0, -6}, p->wisp = false;
        return true;
    }
    if ((lx == c - 2 || lx == c + 2) && ly == c) return p->add(atlas::town(tt::Log)), p->shadow = 0.9f, true;
    if (lx == c && ly == c + 2 && (s->seed & 1)) return p->add(atlas::town(tt::Log)), p->shadow = 0.9f, true;
    if ((lx == 1 && ly == 1) || (lx == 1 && ly == 2 && (r & 1))) return p->add(atlas::town(tt::Sack)), p->shadow = 0.6f, true;
    if ((lx == s->w - 2 && ly == 1) || (lx == s->w - 1 && ly == 2 && (r & 1)))
        return p->add(atlas::tile(r & 2 ? t::Crate : t::Barrel)), p->shadow = 0.8f, true;
    return false;
}
bool ruin(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
    int arch = 1 + int(s->seed % uint32_t(s->w - 3));
    if (ly == 1) {
        if (lx == arch || lx == arch + 1) {
            p->add(atlas::town(lx == arch ? tt::ArchLeft : tt::ArchRight));
            p->add(atlas::town(tt::Bricks), {0, -16});
        } else if (r % 10 < 7) {
            p->add(atlas::town(tt::Bricks));
            if (r % 10 < 3) p->add(atlas::town(tt::Bricks), {0, -16});
        } else {
            return false;
        }
        p->shadow = 1.2f;
        return true;
    }
    if (ly < 2) return false;
    if (r % 11 == 0) return p->add(atlas::town(tt::Bricks)), p->shadow = 1.0f, true;
    if (r % 13 == 0) return p->add(atlas::town(tt::Mushrooms)), p->light = {0.03f, 0.12f, 0.10f, 1}, true;
    return false;
}

const StructureDef *def(const Structure *s) { return &STRUCTURES[int(s->kind)]; }
bool inside(const Structure *s, int lx, int ly) { return lx >= 0 && ly >= 0 && lx < s->w && ly < s->h; }

} // namespace

const StructureDef STRUCTURES[int(StructureKind::Count)] = {
    {"COTTAGE", {1.0f, 0.62f, 0.25f, 1}, sizeSquare, nullptr, cottageGround, cottage},
    {"GRAVEYARD", {0.62f, 0.55f, 1.0f, 1}, sizeGraveyard, nullptr, nullptr, graveyard},
    {"SHRINE", {0.3f, 0.9f, 1.0f, 1}, sizeSquare, nullptr, shrineGround, shrine},
    {"CAMP", {1.0f, 0.35f, 0.3f, 1}, sizeCamp, campDirt, nullptr, camp},
    {"RUIN", {0.6f, 0.9f, 0.4f, 1}, sizeRuin, ruinDirt, nullptr, ruin},
};

bool structureDirt(const Structure *s, int lx, int ly) {
    const StructureDef *d = def(s);
    return inside(s, lx, ly) && d->dirt && d->dirt(s, lx, ly);
}
bool structureGround(const Structure *s, int lx, int ly, Sprite *out) {
    const StructureDef *d = def(s);
    return inside(s, lx, ly) && d->ground && d->ground(s, lx, ly, out);
}
bool structureProp(const Structure *s, int lx, int ly, Prop *p) {
    uint32_t r = hash2(int(s->seed) ^ (lx * 92821), ly * 68917 + 5);
    return inside(s, lx, ly) && def(s)->prop(s, lx, ly, r, p);
}

const char *Structure::name() const { return STRUCTURES[int(kind)].name; }
Color Structure::debugColor() const { return STRUCTURES[int(kind)].debug; }
