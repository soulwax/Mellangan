#pragma once

// Regions inside res/atlas.png. Layout is produced by tools/make_assets.py.

struct Sprite {
    float x, y, w, h;
};

namespace atlas {

// Kenney Tiny Dungeon
constexpr Sprite tile(int index) {
    return {float((index % 12) * 18 + 1), float((index / 12) * 18 + 1), 16, 16};
}
// Kenney Tiny Town, packed to the right of Tiny Dungeon
constexpr Sprite town(int index) {
    Sprite s = tile(index);
    s.x += 12 * 18;
    return s;
}

constexpr Sprite Glow{0, 200, 64, 64};
constexpr Sprite Ring{64, 200, 64, 64};
constexpr Sprite Slash{128, 200, 24, 48};
constexpr Sprite Shadow{176, 200, 16, 8};
constexpr Sprite Gems[3] = {{197, 201, 7, 7}, {209, 201, 7, 7}, {221, 201, 7, 7}};
constexpr Sprite Bolt{232, 200, 10, 10};
constexpr Sprite Dot{244, 200, 8, 8};
constexpr Sprite White{197, 213, 2, 2};
constexpr Sprite digit(int d) { return {float(176 + d * 5), 220, 5, 7}; }

namespace tt {
constexpr int Grass = 0, GrassTufts = 1, GrassFlowers = 2;
// Tall trees are a crown stacked on a trunk.
constexpr int AutumnCrown = 3, Crown = 4, AutumnTrunk = 15, Trunk = 16;
constexpr int Bush = 5, Sprouts = 17, AutumnPine = 27, Pine = 28, Mushrooms = 29;
constexpr int DirtTopLeft = 12; // 3x3 grass-to-dirt edge set, centre at DirtTopLeft + 13
constexpr int Sign = 83, Beehive = 94, Log = 106, Sack = 107;
constexpr int Bomb = 105, Key = 117, Bow = 118, Arrow = 119, Scythe = 129;
constexpr int WellRoof = 92, Well = 104; // roof stacks on the well
} // namespace tt

namespace t {
constexpr int Wizard = 84;
constexpr int Bat = 120, Ghost = 121, Spider = 122, Rat = 123, Scorpion = 124;
constexpr int Slime = 108, Cyclops = 109, Demon = 110, Cultist = 111, Knight = 96;
constexpr int Sand = 48, SandSpeckleA = 49, SandSpeckleB = 51, SandRocks = 42;
constexpr int GraveCross = 64, GraveStone = 65, Barrel = 82, Crystal = 56;
constexpr int Staff = 129, Sword = 104, Shield = 102, Dagger = 103, Axe = 118;
constexpr int PotionGreen = 114, PotionRed = 115, PotionBlue = 116, Hammer = 117, Coin = 101;
constexpr int PotionGrey = 113, VialGreen = 126, VialBlue = 128, StormStaff = 130;
constexpr int Chest = 89;
} // namespace t

} // namespace atlas
