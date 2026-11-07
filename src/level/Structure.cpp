#include "level/Structure.hpp"

#include "level/structure/Layouts.hpp"

namespace {

using namespace layout;

const StructureDef *def(const Structure *s) { return &STRUCTURES[int(s->kind)]; }

bool inside(const Structure *s, int lx, int ly) {
  return lx >= 0 && ly >= 0 && lx < s->w && ly < s->h;
}

constexpr StructureDef landmark(const char *name, Color debug) {
  return {name, debug, sizeLandmark, landmarkDirt, landmarkGround, landmarkProp};
}

} // namespace

const StructureDef STRUCTURES[int(StructureKind::Count)] = {
    {"COTTAGE", {1.0f, 0.62f, 0.25f, 1}, sizeCottage, nullptr, cottageGround, cottage},
    {"GRAVEYARD", {0.62f, 0.55f, 1.0f, 1}, sizeGraveyard, nullptr, nullptr, graveyard},
    {"SHRINE", {0.3f, 0.9f, 1.0f, 1}, sizeSquare, nullptr, shrineGround, shrine},
    {"CAMP", {1.0f, 0.35f, 0.3f, 1}, sizeCamp, campDirt, nullptr, camp},
    {"RUIN", {0.6f, 0.9f, 0.4f, 1}, sizeRuin, ruinDirt, nullptr, ruin},
    {"CASTLE", {0.85f, 0.88f, 1.0f, 1}, sizeCastle, nullptr, castleGround, castle},
    {"STONES", {0.45f, 0.95f, 0.85f, 1}, sizeStones, stonesDirt, nullptr, stones},
    {"MINE", {0.95f, 0.8f, 0.3f, 1}, sizeMine, mineDirt, mineGround, mine},
    landmark("ANCIENT TREE", {0.5f, 1, 0.45f, 1}),
    landmark("FALLEN STAR", {1, 0.95f, 0.6f, 1}),
    landmark("COLOSSUS", {0.9f, 0.86f, 0.76f, 1}),
    landmark("MONOLITH", {0.4f, 0.75f, 1, 1}),
    landmark("CRYSTAL SPIRE", {0.75f, 0.55f, 1, 1}),
    landmark("GREAT MUSHROOM", {0.95f, 0.35f, 0.35f, 1}),
    landmark("SHIPWRECK", {0.75f, 0.55f, 0.35f, 1}),
    landmark("ANCIENT GATE", {0.6f, 0.45f, 1, 1}),
    landmark("EMBER FORGE", {1, 0.5f, 0.2f, 1}),
    landmark("FROZEN GIANT", {0.7f, 0.9f, 1, 1}),
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

Sprite masonry(const Structure *s, int index) {
  return s->masonry && index >= 36 ? atlas::masonry(s->masonry - 1, index)
                                   : atlas::town(index);
}

const char *Structure::name() const { return STRUCTURES[int(kind)].name; }
Color Structure::debugColor() const { return STRUCTURES[int(kind)].debug; }
