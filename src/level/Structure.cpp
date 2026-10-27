#include "level/Structure.hpp"

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;

void sizeSquare(uint32_t, int *w, int *h) { *w = 5, *h = 5; }
void sizeCottage(uint32_t r, int *w, int *h) {
  *w = 5 + int((r >> 13) % 3), *h = 5;
}

struct House {
  int width, door, chimney, storeys;
  bool red, stone, gable, lit;
};

House houseOf(const Structure *s) {
  uint32_t k = s->seed;
  House h{};
  h.width = s->w - 2;
  h.red = k & 1, h.stone = k & 2, h.lit = (k >> 5) % 3 != 0;
  h.storeys = h.width >= 4 && (k >> 9 & 1) ? 2 : 1;
  h.gable = k >> 10 & 1;
  h.door = h.width % 2 ? h.width / 2 : 1 + int(k >> 11 & 1);
  h.chimney = 1 + int((k >> 12) % uint32_t(h.width - 2));
  if (h.gable && h.chimney == h.door)
    h.chimney = -1;
  return h;
}
void sizeGraveyard(uint32_t r, int *w, int *h) {
  *w = 7 + 2 * ((r >> 15) & 1), *h = 6 + ((r >> 16) & 1);
}
void sizeCamp(uint32_t, int *w, int *h) { *w = 7, *h = 7; }
void sizeCastle(uint32_t r, int *w, int *h) {
  *w = 10 + 2 * int((r >> 13) & 1), *h = 10;
}

bool castleCourt(const Structure *s, int lx, int ly) {
  return lx >= 3 && lx < s->w - 3 && ly >= 3 && ly <= s->h - 2;
}

bool castleGround(const Structure *s, int lx, int ly, Sprite *out) {
  int gate = s->w / 2 - 1;
  if (!castleCourt(s, lx, ly) &&
      !(ly == s->h - 1 && (lx == gate || lx == gate + 1)))
    return false;
  *out = atlas::town(tt::Path);
  return true;
}

bool castle(const Structure *s, int lx, int ly, uint32_t, Prop *p) {
  int w = s->w, back = 2, front = s->h - 2, gate = w / 2 - 1;
  int side = lx < 3 ? lx : lx >= w - 3 ? lx - (w - 3) : -1;
  bool arch = s->seed & 1,
       gateHere = ly == front && (lx == gate || lx == gate + 1);
  p->shadow = 1.4f, p->solid = 8;
  if (ly == back || ly == front) {
    bool slit = side == 1 || (side < 0 && (lx - 3) % 2),
         lit = slit && ((s->seed >> (lx + ly)) & 2);
    int yo = 0;
    if (ly == front)
      p->add(atlas::town(gateHere ? (arch ? tt::OpenGate : tt::Portcullis) +
                                        (lx - gate)
                                  : tt::Bricks)),
          yo = -16;
    p->add(atlas::town(slit ? tt::ArrowSlit : tt::Bricks), yo);
    if (side >= 0)
      p->add(atlas::town(tt::TowerTop + 24 + side), yo - 16),
          p->add(atlas::town(tt::TowerTop + side), yo - 32);
    else
      p->add(atlas::town(tt::WallTop + (lx == 3       ? 0
                                        : lx == w - 4 ? 2
                                                      : 1)),
             yo - 16);
    if (gateHere)
      p->light = {0.55f, 0.27f, 0.07f, 1}, p->lightYo = -6, p->lightSize = 90,
      p->wisp = false;
    else if (lit)
      p->light = {0.30f, 0.17f, 0.05f, 1}, p->lightYo = yo - 8,
      p->lightSize = 48, p->wisp = false;
    if (gateHere && arch)
      p->solid = 0;
    return true;
  }
  if (side < 0 || ly <= back || ly >= front - 3)
    return false;
  p->add(atlas::town(tt::TowerTop + 12 + side));
  p->shadow = 0;
  return true;
}

void sizeRuin(uint32_t r, int *w, int *h) { *w = 6 + ((r >> 15) & 1), *h = 4; }

bool campDirt(const Structure *s, int lx, int ly) {
  float dx = lx - (s->w - 1) * 0.5f, dy = ly - (s->h - 1) * 0.5f;
  return dx * dx + dy * dy < 2.7f * 2.7f;
}
bool ruinDirt(const Structure *, int, int) { return true; }

bool shrineGround(const Structure *s, int lx, int ly, Sprite *out) {
  int row = ly == 0          ? 0
            : ly == s->h - 1 ? 2
                             : 1,
      col = lx == 0          ? 0
            : lx == s->w - 1 ? 2
                             : 1;
  *out = atlas::town(tt::PlazaTopLeft + row * 12 + col);
  return true;
}
bool cottageGround(const Structure *s, int lx, int ly, Sprite *out) {
  if (ly < 3 || lx != houseOf(s).door + 1)
    return false;
  *out = atlas::town(tt::Path);
  return true;
}

bool cottage(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
  House h = houseOf(s);
  int c = lx - 1;
  if (ly == 2 && c >= 0 && c < h.width) {
    int roof = h.red ? tt::RedRoof : tt::GreyRoof, side = c == 0 ? 0
                                                          : c == h.width - 1
                                                              ? 2
                                                              : 1;
    int wall[3] = {h.stone ? tt::StoneWallLeft : tt::WoodWallLeft,
                   h.stone ? tt::StoneWall : tt::WoodWall,
                   h.stone ? tt::StoneWallRight : tt::WoodWallRight};
    int window = h.stone ? tt::StoneWindow : tt::WoodWindow,
        doorTile = h.stone ? tt::StoneDoor : tt::WoodDoor;
    bool inside = side == 1, door = c == h.door;
    p->add(atlas::town(!inside ? wall[side] : door ? doorTile : window));
    int yo = -16;
    if (h.storeys == 2)
      p->add(atlas::town(inside && c % 2 ? window : wall[side]), yo), yo -= 16;
    p->add(atlas::town(h.gable && door ? roof + tt::RoofGable
                                       : roof + tt::RoofEaves + side),
           yo);
    p->add(atlas::town(c == h.chimney ? roof + tt::RoofChimney : roof + side),
           yo - 16);
    p->shadow = 1.4f, p->solid = 8;
    if (inside && !door && h.lit)
      p->light = {0.34f, 0.2f, 0.06f, 1}, p->lightYo = -8, p->lightSize = 56,
      p->wisp = false;
    return true;
  }
  if (lx == 0 && ly == 3)
    return p->add(atlas::tile(r & 1 ? t::Barrel : t::Crate)),
           p->shadow = 0.8f, p->solid = 5, true;
  if (lx == s->w - 1 && ly == 3) {
    if (s->seed & 8)
      return p->add(atlas::town(tt::Beehive)), p->shadow = 0.8f, p->solid = 4,
                                               true;
    p->add(atlas::town(tt::Well)), p->add(atlas::town(tt::WellRoof), -16),
        p->shadow = 1.0f, p->solid = 6;
    return true;
  }
  if (lx == 1 && ly == 4 && (s->seed & 16))
    return p->add(atlas::town(tt::Sign)), p->shadow = 0.6f, p->solid = 3, true;
  return false;
}
bool graveyard(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
  int right = s->w - 1, bottom = s->h - 1, gate = s->w / 2;
  p->solid = 6;
  if (ly == 0)
    return p->add(atlas::town(lx == 0       ? tt::FenceTopLeft
                              : lx == right ? tt::FenceTopRight
                                            : tt::FenceTop)),
           true;
  if (ly == bottom) {
    if (lx == gate)
      return false;
    int f = lx == 0          ? tt::FenceBottomLeft
            : lx == right    ? tt::FenceBottomRight
            : lx == gate - 1 ? tt::FenceEndRight
            : lx == gate + 1 ? tt::FenceEndLeft
                             : tt::FenceRail;
    return p->add(atlas::town(f)), true;
  }
  if (lx == 0 || lx == right)
    return p->add(atlas::town(lx == 0 ? tt::FenceLeft : tt::FenceRight)), true;
  if (lx % 2 == 1 && ly % 2 == 0 && r % 5 != 0) {
    p->add(atlas::tile(r % 3 == 0 ? t::GraveCross : t::GraveStone));
    if (r % 4 == 0)
      p->light = {0.07f, 0.15f, 0.32f, 1};
    return true;
  }
  return false;
}
bool shrine(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
  bool corner = (lx == 0 || lx == s->w - 1) && (ly == 0 || ly == s->h - 1);
  if (corner) {
    p->add(atlas::tile(t::Pillar));
    if (r % 3 != 0)
      p->add(atlas::tile(t::Pillar), -16);
    p->shadow = 1.0f, p->solid = 6;
    return true;
  }
  if (lx == s->w / 2 && ly == s->h / 2) {
    p->add(atlas::tile(t::Crystal)), p->shadow = 0.7f, p->solid = 4;
    p->light = {0.06f, 0.22f, 0.38f, 1}, p->lightSize = 130, p->lightYo = -12;
    return true;
  }
  return false;
}
bool camp(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
  int c = s->w / 2;
  p->solid = 5;
  if (lx == c && ly == c) {
    p->add(atlas::town(tt::Log));
    p->light = {0.55f, 0.27f, 0.07f, 1}, p->lightSize = 150, p->lightYo = -6,
    p->wisp = false;
    return true;
  }
  if ((lx == c - 2 || lx == c + 2) && ly == c)
    return p->add(atlas::town(tt::Log)), p->shadow = 0.9f, true;
  if (lx == c && ly == c + 2 && (s->seed & 1))
    return p->add(atlas::town(tt::Log)), p->shadow = 0.9f, true;
  if ((lx == 1 && ly == 1) || (lx == 1 && ly == 2 && (r & 1)))
    return p->add(atlas::town(tt::Sack)), p->shadow = 0.6f, true;
  if ((lx == s->w - 2 && ly == 1) || (lx == s->w - 1 && ly == 2 && (r & 1)))
    return p->add(atlas::tile(r & 2 ? t::Crate : t::Barrel)), p->shadow = 0.8f,
                                                              true;
  return false;
}
bool ruin(const Structure *s, int lx, int ly, uint32_t r, Prop *p) {
  int arch = 1 + int(s->seed % uint32_t(s->w - 3));
  if (ly == 1) {
    if (lx == arch || lx == arch + 1) {
      p->add(atlas::town(lx == arch ? tt::ArchLeft : tt::ArchRight));
      p->add(atlas::town(tt::Bricks), -16);
    } else if (r % 10 < 7) {
      p->add(atlas::town(tt::Bricks)), p->solid = 8;
      if (r % 10 < 3)
        p->add(atlas::town(tt::Bricks), -16);
    } else {
      return false;
    }
    p->shadow = 1.2f;
    return true;
  }
  if (ly < 2)
    return false;
  if (r % 11 == 0)
    return p->add(atlas::town(tt::Bricks)), p->shadow = 1.0f, p->solid = 7,
                                            true;
  if (r % 13 == 0)
    return p->add(atlas::town(tt::Mushrooms)),
           p->light = {0.03f, 0.12f, 0.10f, 1}, true;
  return false;
}

const StructureDef *def(const Structure *s) {
  return &STRUCTURES[int(s->kind)];
}
bool inside(const Structure *s, int lx, int ly) {
  return lx >= 0 && ly >= 0 && lx < s->w && ly < s->h;
}

} // namespace

const StructureDef STRUCTURES[int(StructureKind::Count)] = {
    {"COTTAGE",
     {1.0f, 0.62f, 0.25f, 1},
     sizeCottage,
     nullptr,
     cottageGround,
     cottage},
    {"GRAVEYARD",
     {0.62f, 0.55f, 1.0f, 1},
     sizeGraveyard,
     nullptr,
     nullptr,
     graveyard},
    {"SHRINE",
     {0.3f, 0.9f, 1.0f, 1},
     sizeSquare,
     nullptr,
     shrineGround,
     shrine},
    {"CAMP", {1.0f, 0.35f, 0.3f, 1}, sizeCamp, campDirt, nullptr, camp},
    {"RUIN", {0.6f, 0.9f, 0.4f, 1}, sizeRuin, ruinDirt, nullptr, ruin},
    {"CASTLE",
     {0.85f, 0.88f, 1.0f, 1},
     sizeCastle,
     nullptr,
     castleGround,
     castle},
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

bool cottageChimney(const Structure *s, int *lx, int *height) {
  House h = houseOf(s);
  if (h.chimney < 0)
    return false;
  *lx = h.chimney + 1, *height = 16 * (h.storeys + 2);
  return true;
}

const char *Structure::name() const { return STRUCTURES[int(kind)].name; }
Color Structure::debugColor() const { return STRUCTURES[int(kind)].debug; }
