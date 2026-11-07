#include "level/structure/Layouts.hpp"

#include "level/Noise.h"

namespace layout {

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;
namespace fr = atlas::fr;

const int RING[8][2] = {{3, 0}, {5, 1}, {6, 3}, {5, 5},
                        {3, 6}, {1, 5}, {0, 3}, {1, 1}};

int edge(int v, int last) { return v == 0 ? 0 : v == last ? 2 : 1; }

} // namespace

void sizeSquare(uint32_t, int *w, int *h) { *w = 5, *h = 5; }

bool shrineGround(Site s, int lx, int ly, Sprite *out) {
  *out = masonry(s, tt::PlazaTopLeft + edge(ly, s->h - 1) * 12 + edge(lx, s->w - 1));
  return true;
}

bool shrine(Site s, int lx, int ly, uint32_t r, Prop *p) {
  bool corner = (lx == 0 || lx == s->w - 1) && (ly == 0 || ly == s->h - 1);
  if (corner) {
    p->add(atlas::tile(t::Pillar));
    if (r % 3 != 0)
      p->add(atlas::tile(t::Pillar), -16);
    return p->body(1.0f, 6);
  }
  if (lx == s->w / 2 && ly == s->h / 2) {
    p->piece(atlas::tile(t::Crystal), 0.7f, 4);
    p->lamp({0.06f, 0.22f, 0.38f, 1}, 130, -12);
    return true;
  }
  return false;
}

void sizeRuin(uint32_t r, int *w, int *h) { *w = 6 + ((r >> 15) & 1), *h = 4; }

bool ruinDirt(Site s, int lx, int ly) {
  float dx = (lx - (s->w - 1) * 0.5f) / (s->w * 0.5f),
        dy = (ly - (s->h - 1) * 0.5f) / (s->h * 0.5f + 0.3f), d = dx * dx + dy * dy;
  return d < 0.35f ||
         d < 0.75f + 0.5f * noise_fbm((s->x + lx) * 0.35f, (s->y + ly) * 0.35f, 2, 1217);
}

bool ruin(Site s, int lx, int ly, uint32_t r, Prop *p) {
  int arch = 1 + int(s->seed % uint32_t(s->w - 3));
  if (ly == 1) {
    if (lx == arch || lx == arch + 1) {
      p->add(masonry(s, lx == arch ? tt::ArchLeft : tt::ArchRight));
      p->add(masonry(s, tt::Bricks), -16);
    } else if (r % 10 < 7) {
      p->add(masonry(s, tt::Bricks)), p->solid = 8;
      if (r % 10 < 3)
        p->add(masonry(s, tt::Bricks), -16);
    } else {
      return false;
    }
    p->shadow = 1.2f;
    return true;
  }
  if (ly < 2)
    return false;
  if (r % 11 == 0)
    return p->piece(masonry(s, tt::Bricks), 1.0f, 7);
  if (r % 13 == 0)
    return p->add(masonry(s, tt::Mushrooms)), p->lamp({0.03f, 0.12f, 0.10f, 1}), true;
  return false;
}

void sizeStones(uint32_t, int *w, int *h) { *w = 7, *h = 7; }

bool stonesDirt(Site, int lx, int ly) {
  int dx = lx - 3, dy = ly - 3;
  return dx * dx + dy * dy <= 5;
}

bool stones(Site s, int lx, int ly, uint32_t r, Prop *p) {
  if (lx == 3 && ly == 3) {
    p->piece(atlas::frontier(fr::Altar), 0.9f, 6);
    p->lamp({0.06f, 0.26f, 0.34f, 1}, 120, -8);
    return true;
  }
  for (int i = 0; i < 8; i++) {
    if (RING[i][0] != lx || RING[i][1] != ly)
      continue;
    if ((s->seed >> (i * 2) & 3) == 0)
      return p->piece(atlas::frontier(fr::ToppledStone), 0.8f, 5);
    p->add(atlas::frontier(fr::RuneStoneLow));
    p->add(atlas::frontier(fr::RuneStoneHigh), -16);
    p->body(1.2f, 5);
    if (r % 3 == 0)
      p->lamp({0.04f, 0.16f, 0.22f, 1}, 60, -18);
    return true;
  }
  return false;
}

void sizeMine(uint32_t, int *w, int *h) { *w = 7, *h = 6; }

bool mineDirt(Site, int, int ly) { return ly >= 1; }

bool mineGround(Site, int lx, int ly, Sprite *out) {
  if (lx != 3 || ly < 2)
    return false;
  *out = atlas::frontier(fr::RailDown);
  return true;
}

bool mine(Site s, int lx, int ly, uint32_t r, Prop *p) {
  static const int MOUTH[3][2] = {{fr::MineLeftLow, fr::MineLeftHigh},
                                  {fr::MineMouthLow, fr::MineMouthHigh},
                                  {fr::MineRightLow, fr::MineRightHigh}};
  if (ly == 1 && lx >= 2 && lx <= 4) {
    p->add(atlas::frontier(MOUTH[lx - 2][0]));
    p->add(atlas::frontier(MOUTH[lx - 2][1]), -16);
    p->body(1.4f, 8);
    if (lx == 2)
      p->fire({0.45f, 0.24f, 0.06f, 1}, 70, -14);
    return true;
  }
  if (lx == 3 && ly == 4)
    return p->piece(atlas::frontier(fr::MineCart), 0.8f, 5);
  if (lx == 5 && ly == 2) {
    p->piece(atlas::frontier(fr::LanternPost), 0.6f, 3);
    p->fire({0.5f, 0.28f, 0.07f, 1}, 90, -6);
    return true;
  }
  if ((lx == 1 && ly == 3) || (lx == 5 && ly == 4 && (r & 1)))
    return p->piece(atlas::frontier(fr::OrePile), 0.8f, 5);
  if (lx == 1 && ly == 5 && (s->seed & 2))
    return p->piece(atlas::frontier(fr::Timber), 0.8f, 0);
  if (lx == 6 && ly == 3)
    return p->piece(atlas::tile(r & 2 ? t::Crate : t::Barrel), 0.8f, 5);
  return false;
}

} // namespace layout
