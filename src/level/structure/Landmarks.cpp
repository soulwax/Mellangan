#include "level/structure/Layouts.hpp"

namespace layout {

namespace {

struct LandmarkLook {
  Color light;
  float size;
  int yo;
  float solid, shadow;
};

const LandmarkLook LOOKS[] = {{{0.6f, 0.52f, 0.2f, 1}, 140, -40, 7, 2.2f},
                              {{0.95f, 0.85f, 0.5f, 1}, 180, -14, 7, 0},
                              {{0.32f, 0.42f, 0.6f, 1}, 100, -20, 6, 1.8f},
                              {{0.25f, 0.6f, 0.95f, 1}, 130, -30, 5, 1.2f},
                              {{0.55f, 0.35f, 0.9f, 1}, 150, -30, 7, 1.4f},
                              {{0.5f, 0.3f, 0.35f, 1}, 110, -40, 6, 2.4f},
                              {{0.45f, 0.38f, 0.22f, 1}, 70, -16, 7, 2},
                              {{0.5f, 0.3f, 0.95f, 1}, 160, -24, 6, 1.2f},
                              {{0.95f, 0.45f, 0.12f, 1}, 150, -10, 7, 1.4f},
                              {{0.45f, 0.65f, 0.9f, 1}, 120, -24, 7, 1.6f}};

} // namespace

void sizeLandmark(uint32_t, int *w, int *h) { *w = 7, *h = 7; }

bool landmarkDirt(Site s, int lx, int ly) {
  int dx = lx - 3, dy = ly - 4;
  if (s->kind == StructureKind::FallenStar)
    return dx * dx + dy * dy * 2 <= 7;
  if (s->kind == StructureKind::Colossus)
    return dx * dx + dy * dy * 3 <= 4;
  if (s->kind == StructureKind::EmberForge)
    return dx * dx + dy * dy <= 2;
  return false;
}

bool landmarkGround(Site s, int lx, int ly, Sprite *out) {
  if ((s->kind != StructureKind::Monolith &&
       s->kind != StructureKind::AncientGate) ||
      lx < 2 || lx > 4 || ly < 3 || ly > 5)
    return false;
  *out = masonry(s, atlas::tt::PlazaTopLeft + (ly - 3) * 12 + (lx - 2));
  return true;
}

bool landmarkProp(Site s, int lx, int ly, uint32_t, Prop *p) {
  if (ly != 4 || lx < 2 || lx > 4)
    return false;
  int index = int(s->kind) - int(StructureKind::AncientTree), col = lx - 2;
  const LandmarkLook &look = LOOKS[index];
  for (int row = 3; row >= 0; row--)
    p->add(atlas::landmark(index, row, col), -(3 - row) * 16);
  if (col == 1)
    p->body(look.shadow, look.solid), p->lamp(look.light, look.size, look.yo);
  else if (s->kind == StructureKind::Colossus)
    p->solid = 6;
  return true;
}

} // namespace layout
