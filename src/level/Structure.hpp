#pragma once

#include "level/Prop.hpp"

enum class StructureKind {
  Cottage,
  Graveyard,
  Shrine,
  Camp,
  Ruin,
  Castle,
  Stones,
  Mine,
  Count
};
struct Structure {
  StructureKind kind;
  int x, y, w, h; // footprint in tiles
  int rx, ry;     // owning region
  uint32_t seed;

  bool contains(int tx, int ty, int margin = 0) const {
    return tx >= x - margin && ty >= y - margin && tx < x + w + margin &&
           ty < y + h + margin;
  }
  double centerX() const { return (x + w * 0.5) * 16; }
  double centerY() const { return (y + h * 0.5) * 16; }
  const char *name() const;
  Color debugColor() const;
};
struct StructureDef {
  const char *name;
  Color debug;
  void (*size)(uint32_t roll, int *w, int *h);
  bool (*dirt)(const Structure *s, int lx, int ly);
  bool (*ground)(const Structure *s, int lx, int ly, Sprite *out);
  bool (*prop)(const Structure *s, int lx, int ly, uint32_t roll, Prop *p);
};
extern const StructureDef STRUCTURES[int(StructureKind::Count)];

bool structureDirt(const Structure *s, int lx, int ly);
bool structureGround(const Structure *s, int lx, int ly, Sprite *out);
bool structureProp(const Structure *s, int lx, int ly, Prop *p);
bool cottageChimney(const Structure *s, int *lx, int *height);
