#pragma once

#include "level/Prop.hpp"

struct Feature;
typedef bool (*FeaturePlace)(const Feature *f, uint32_t roll, Prop *p);
struct Feature {
    const char *name;
    FeaturePlace place;
    int tile, top; // atlas indices; top stacks one tile up, -1 for none
    bool town;     // Tiny Town sheet, else Tiny Dungeon
    float shadow, solid; // solid: collider radius at the foot, 0 to walk through
    Color light;
    bool winter = false;
};

bool placeStack(const Feature *f, uint32_t roll, Prop *p);
bool placeGrave(const Feature *f, uint32_t roll, Prop *p);

namespace feature {
extern const Feature oak, autumnOak, pine, autumnPine, bush, snowOak, snowPine, snowBush;
extern const Feature grave, mushrooms, sprouts, log, beehive, sign, sack, crystal, well, barrel, crate;
} // namespace feature
