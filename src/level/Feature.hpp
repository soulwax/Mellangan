#pragma once

#include "level/Prop.hpp"

// A free-standing prop scattered over open ground. Biomes reference these by address.
struct Feature;
typedef bool (*FeaturePlace)(const Feature *f, uint32_t roll, Prop *p);
struct Feature {
    const char *name;
    FeaturePlace place;
    int tile, top; // atlas indices; top stacks one tile up, -1 for none
    bool town;     // Tiny Town sheet, else Tiny Dungeon
    float shadow;
    Color light;
};

bool placeStack(const Feature *f, uint32_t roll, Prop *p);
bool placeGrave(const Feature *f, uint32_t roll, Prop *p);

namespace feature {
extern const Feature oak, autumnOak, pine, autumnPine, bush;
extern const Feature grave, mushrooms, sprouts, log, beehive, sign, sack, crystal, well, barrel, crate;
} // namespace feature
