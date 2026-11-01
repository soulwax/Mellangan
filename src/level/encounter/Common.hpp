#pragma once

#include "gfx/Screen.hpp"
#include "level/Structure.hpp"

class Level;

namespace encounter {

typedef const Structure *Site;

double atX(Site s, int lx);
double atY(Site s, int ly);
double footY(Site s);
void smoke(Level *level, double x, double y, double z);
void drift(Level *level, double x, double y, int icon, Color c, float rise);
void quake(Level *level, double x, double y, double r, float damage, float force);

void cottage(Level *level, Site s);
void hearth(Level *level, Site s);
void graveyard(Level *level, Site s);
void shrine(Level *level, Site s);
void camp(Level *level, Site s);
void campfire(Level *level, Site s);
void ruin(Level *level, Site s);
void castle(Level *level, Site s);
void braziers(Level *level, Site s);
void stones(Level *level, Site s);
void hum(Level *level, Site s);
void mine(Level *level, Site s);

void ancientTree(Level *level, Site s);
void leaves(Level *level, Site s);
void fallenStar(Level *level, Site s);
void starlight(Level *level, Site s);
void colossus(Level *level, Site s);
void bones(Level *level, Site s);
void monolith(Level *level, Site s);
void runes(Level *level, Site s);
void spire(Level *level, Site s);
void shimmer(Level *level, Site s);
void mushroom(Level *level, Site s);
void spores(Level *level, Site s);
void wreck(Level *level, Site s);
void gate(Level *level, Site s);
void swirl(Level *level, Site s);
void forge(Level *level, Site s);
void coals(Level *level, Site s);
void frozen(Level *level, Site s);
void frost(Level *level, Site s);

} // namespace encounter
