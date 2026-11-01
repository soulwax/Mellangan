#include "level/structure/Layouts.hpp"

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;

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

bool castleCourt(const Structure *s, int lx, int ly) {
  return lx >= 3 && lx < s->w - 3 && ly >= 3 && ly <= s->h - 2;
}

} // namespace

bool cottageChimney(const Structure *s, int *lx, int *height) {
  House h = houseOf(s);
  if (h.chimney < 0)
    return false;
  *lx = h.chimney + 1, *height = 16 * (h.storeys + 2);
  return true;
}

namespace layout {

void sizeCottage(uint32_t r, int *w, int *h) {
  *w = 5 + int((r >> 13) % 3), *h = 5;
}

bool cottageGround(Site s, int lx, int ly, Sprite *out) {
  if (ly < 3 || lx != houseOf(s).door + 1)
    return false;
  *out = atlas::town(tt::Path);
  return true;
}

bool cottage(Site s, int lx, int ly, uint32_t r, Prop *p) {
  House h = houseOf(s);
  int c = lx - 1;
  if (ly == 2 && c >= 0 && c < h.width) {
    int roof = h.red ? tt::RedRoof : tt::GreyRoof,
        side = c == 0 ? 0 : c == h.width - 1 ? 2 : 1;
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
    p->body(1.4f, 8);
    if (inside && !door && h.lit)
      p->fire({0.34f, 0.2f, 0.06f, 1}, 56, -8);
    return true;
  }
  if (lx == 0 && ly == 3)
    return p->piece(atlas::tile(r & 1 ? t::Barrel : t::Crate), 0.8f, 5);
  if (lx == s->w - 1 && ly == 3) {
    if (s->seed & 8)
      return p->piece(atlas::town(tt::Beehive), 0.8f, 4);
    p->add(atlas::town(tt::Well)), p->add(atlas::town(tt::WellRoof), -16);
    return p->body(1.0f, 6);
  }
  if (lx == 1 && ly == 4 && (s->seed & 16))
    return p->piece(atlas::town(tt::Sign), 0.6f, 3);
  return false;
}

void sizeGraveyard(uint32_t r, int *w, int *h) {
  *w = 7 + 2 * ((r >> 15) & 1), *h = 6 + ((r >> 16) & 1);
}

bool graveyard(Site s, int lx, int ly, uint32_t r, Prop *p) {
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
      p->lamp({0.07f, 0.15f, 0.32f, 1});
    return true;
  }
  return false;
}

void sizeCamp(uint32_t, int *w, int *h) { *w = 7, *h = 7; }

bool campDirt(Site s, int lx, int ly) {
  float dx = lx - (s->w - 1) * 0.5f, dy = ly - (s->h - 1) * 0.5f;
  return dx * dx + dy * dy < 2.7f * 2.7f;
}

bool camp(Site s, int lx, int ly, uint32_t r, Prop *p) {
  int c = s->w / 2;
  p->solid = 5;
  if (lx == c && ly == c) {
    p->add(atlas::town(tt::Log));
    p->fire({0.55f, 0.27f, 0.07f, 1}, 150, -6);
    return true;
  }
  if ((lx == c - 2 || lx == c + 2) && ly == c)
    return p->piece(atlas::town(tt::Log), 0.9f, 5);
  if (lx == c && ly == c + 2 && (s->seed & 1))
    return p->piece(atlas::town(tt::Log), 0.9f, 5);
  if ((lx == 1 && ly == 1) || (lx == 1 && ly == 2 && (r & 1)))
    return p->piece(atlas::town(tt::Sack), 0.6f, 5);
  if ((lx == s->w - 2 && ly == 1) || (lx == s->w - 1 && ly == 2 && (r & 1)))
    return p->piece(atlas::tile(r & 2 ? t::Crate : t::Barrel), 0.8f, 5);
  return false;
}

void sizeCastle(uint32_t r, int *w, int *h) {
  *w = 10 + 2 * int((r >> 13) & 1), *h = 10;
}

bool castleGround(Site s, int lx, int ly, Sprite *out) {
  int gate = s->w / 2 - 1;
  if (!castleCourt(s, lx, ly) &&
      !(ly == s->h - 1 && (lx == gate || lx == gate + 1)))
    return false;
  *out = atlas::town(tt::Path);
  return true;
}

bool castle(Site s, int lx, int ly, uint32_t, Prop *p) {
  int w = s->w, back = 2, front = s->h - 2, gate = w / 2 - 1;
  int side = lx < 3 ? lx : lx >= w - 3 ? lx - (w - 3) : -1;
  bool arch = s->seed & 1,
       gateHere = ly == front && (lx == gate || lx == gate + 1);
  p->body(1.4f, 8);
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
      p->add(atlas::town(tt::WallTop + (lx == 3 ? 0 : lx == w - 4 ? 2 : 1)),
             yo - 16);
    if (gateHere)
      p->fire({0.55f, 0.27f, 0.07f, 1}, 90, -6);
    else if (lit)
      p->fire({0.30f, 0.17f, 0.05f, 1}, 48, yo - 8);
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

} // namespace layout
