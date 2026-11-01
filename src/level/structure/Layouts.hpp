#pragma once

#include "level/Structure.hpp"

namespace layout {

typedef const Structure *Site;

void sizeCottage(uint32_t r, int *w, int *h);
bool cottageGround(Site s, int lx, int ly, Sprite *out);
bool cottage(Site s, int lx, int ly, uint32_t r, Prop *p);
void sizeGraveyard(uint32_t r, int *w, int *h);
bool graveyard(Site s, int lx, int ly, uint32_t r, Prop *p);
void sizeCamp(uint32_t r, int *w, int *h);
bool campDirt(Site s, int lx, int ly);
bool camp(Site s, int lx, int ly, uint32_t r, Prop *p);
void sizeCastle(uint32_t r, int *w, int *h);
bool castleGround(Site s, int lx, int ly, Sprite *out);
bool castle(Site s, int lx, int ly, uint32_t r, Prop *p);

void sizeSquare(uint32_t r, int *w, int *h);
bool shrineGround(Site s, int lx, int ly, Sprite *out);
bool shrine(Site s, int lx, int ly, uint32_t r, Prop *p);
void sizeRuin(uint32_t r, int *w, int *h);
bool ruinDirt(Site s, int lx, int ly);
bool ruin(Site s, int lx, int ly, uint32_t r, Prop *p);
void sizeStones(uint32_t r, int *w, int *h);
bool stonesDirt(Site s, int lx, int ly);
bool stones(Site s, int lx, int ly, uint32_t r, Prop *p);
void sizeMine(uint32_t r, int *w, int *h);
bool mineDirt(Site s, int lx, int ly);
bool mineGround(Site s, int lx, int ly, Sprite *out);
bool mine(Site s, int lx, int ly, uint32_t r, Prop *p);

void sizeLandmark(uint32_t r, int *w, int *h);
bool landmarkDirt(Site s, int lx, int ly);
bool landmarkGround(Site s, int lx, int ly, Sprite *out);
bool landmarkProp(Site s, int lx, int ly, uint32_t r, Prop *p);

} // namespace layout
