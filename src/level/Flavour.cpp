#include "level/Flavour.hpp"

const Flavour FLAVOURS[BIOME_COUNT] = {
    {1, 1, 1, 0, false, ""},
    {1, 0.92f, 1, 0, false, "foes stumble"},
    {1, 1, 1.1f, 0, false, "+10% xp"},
    {1.08f, 1, 1, 0, false, "swift step"},
    {1, 1.08f, 1.25f, 0, false, "+25% xp, restless dead"},
    {0.94f, 0.85f, 1, 0, false, "all slowed"},
    {1, 1, 1, 0.5f, false, "gentle healing"},
    {0.85f, 1, 1, 0, false, "boggy, slow step"},
    {0.95f, 0.9f, 1, 0, false, "heat slows all"},
    {0.95f, 1, 1.15f, 0, false, "+15% xp, thick growth"},
    {1.12f, 1, 1, 0, false, "open plains, swift"},
    {1, 1, 1, 0, true, "embers scorch foes"},
    {1, 1, 1.25f, 0, false, "+25% xp"},
    {1, 1, 1, 0.8f, false, "spores heal"},
    {1, 1.06f, 1.2f, 0, false, "+20% xp, scorched ground"},
    {0.96f, 0.88f, 1, 0, false, "deep needles slow all"},
};

const Omen OMENS[OMEN_COUNT] = {
    {"BLOOD MOON",
     "elites stalk, +50% xp",
     1,
     1,
     1.5f,
     3,
     1,
     {0.95f, 0.3f, 0.3f, 1}},
    {"AURORA", "shrines kindle fast", 1, 1, 1, 1, 2, {0.45f, 1, 0.8f, 1}},
    {"GALE", "swift winds for all", 1.15f, 1.1f, 1, 1, 1, {0.75f, 0.85f, 1, 1}},
};
