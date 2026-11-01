#include "level/Encounter.hpp"

#include "level/Level.hpp"
#include "level/encounter/Common.hpp"
#include "particle/Drift.hpp"
#include "particle/debris/SmokeDebris.hpp"

namespace encounter {

double atX(Site s, int lx) { return (s->x + lx) * 16 + 8.0; }
double atY(Site s, int ly) { return (s->y + ly) * 16 + 12.0; }
double footY(Site s) { return (s->y + 5) * 16.0; }

void smoke(Level *level, double x, double y, double z) {
  if (!Debris::room())
    return;
  auto puff = std::make_unique<SmokeDebris>(x, y, z);
  puff->xa *= 0.15, puff->ya *= 0.05, puff->za = 0.3, puff->life *= 2;
  level->spawn(std::move(puff));
}

void drift(Level *level, double x, double y, int icon, Color c, float rise) {
  if (!Drift::room())
    return;
  Drift *d = level->add<Drift>(x, y, rng.range(-6, 6), rise, icon, c,
                               rng.range(2.5f, 4));
  d->sway = 8, d->scale = 0.8f, d->drag = 0.4f;
}

} // namespace encounter

using namespace encounter;

const EncounterDef ENCOUNTERS[int(StructureKind::Count)] = {
    {"A WARM HEARTH", cottage, hearth},
    {"THE DEAD STIR", graveyard, nullptr},
    {"STAND AND KINDLE THE SHRINE", shrine, nullptr},
    {"AN ABANDONED CAMP", camp, campfire},
    {"SOMETHING GLINTS IN THE RUBBLE", ruin, nullptr},
    {"THE GARRISON WAKES", castle, braziers},
    {"THE STONES HUM", stones, hum},
    {"SOMETHING STIRS BELOW", mine, nullptr},
    {"THE ANCIENT TREE", ancientTree, leaves},
    {"A FALLEN STAR", fallenStar, starlight},
    {"THE COLOSSUS STIRS", colossus, bones},
    {"THE MONOLITH SPEAKS", monolith, runes},
    {"THE CRYSTAL SPIRE", spire, shimmer},
    {"THE GREAT MUSHROOM", mushroom, spores},
    {"A WRECK FULL OF PLUNDER", wreck, nullptr},
    {"THE ANCIENT GATE", gate, swirl},
    {"THE EMBER FORGE", forge, coals},
    {"THE FROZEN GIANT", frozen, frost},
};
