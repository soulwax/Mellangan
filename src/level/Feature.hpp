#pragma once

#include "level/Prop.hpp"

struct Feature;
typedef bool (*FeaturePlace)(const Feature *f, uint32_t roll, Prop *p);
struct Feature {
  const char *name;
  FeaturePlace place;
  int tile, top;       // atlas indices; top stacks one tile up, -1 for none
  bool town;           // Tiny Town sheet, else Tiny Dungeon
  float shadow, solid; // solid: collider radius at the foot, 0 to walk through
  Color light;
  bool winter = false;
  Sprite (*art)(int) = nullptr;
};

bool placeStack(const Feature *f, uint32_t roll, Prop *p);
bool placeGrave(const Feature *f, uint32_t roll, Prop *p);

namespace feature {
extern const Feature oak, autumnOak, pine, autumnPine, bush, snowOak, snowPine,
    snowBush;
extern const Feature pinkOak, whiteOak, lilac, berryBush, dryBush, deadPine,
    spiritPine, mossPine, blueShrooms;
extern const Feature willow, reeds, toadstools, cactus, barrelCactus, palm;
extern const Feature appleTree, autumnBirch, deadOak, weepingWillow, bamboo,
    redwood, treeFern, blueSpruce, joshuaTree, roses, blueberries, hydrangea,
    holly, pumpkins, brownMushrooms, stump, mossyLog, poppies, cornflowers,
    bluebells, bracken, clover, menhir, rockPile, amethyst, skull, driftwood,
    pricklyPear, agave, tumbleweed, snowdrops, frozenShrub, glowshrooms, ivy,
    sage, callaLilies, strawberries, pampas, stoneRing;
extern const Feature birch, frostBirch, cypress, acacia, baobab, mushroomTree,
    jungleTree, charredTree, crystalSpire, fir, snowyFir, fern, frostedFern,
    tallGrass, dryGrass, heather, thornbush, sunflower, tulips, lavender, wheat,
    cattails, daisies, boulder, mossyBoulder, snowyBoulder, sandstone, iceChunk,
    obsidian, pebbles, sandTuft, frostTuft, ashTuft, bogTuft, bloomTuft;
extern const Feature mesa, hoodoo, juniper, taigaSpruce, larch, runeStone,
    ocotillo, redRocks, ribs, clayTuft, lingonberry, lichenRock, cottonGrass,
    needleTuft, shelfStump;
extern const Feature grave, mushrooms, sprouts, log, beehive, sign, sack,
    crystal, well, barrel, crate;
} // namespace feature
