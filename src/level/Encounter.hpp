#pragma once

#include "level/Structure.hpp"

class Level;
struct EncounterDef {
  const char *banner;
  void (*enter)(Level *level, const Structure *s);
  void (*ambient)(Level *level, const Structure *s);
};
extern const EncounterDef ENCOUNTERS[int(StructureKind::Count)];
