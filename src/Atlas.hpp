#pragma once

// Regions inside res/atlas.png. Layout is produced by tools/make_assets.py.

struct Sprite {
    float x, y, w, h;
};

namespace atlas {

constexpr Sprite tile(int index) {
    return {float((index % 12) * 18 + 1), float((index / 12) * 18 + 1), 16, 16};
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

// Kenney Tiny Dungeon tile indices
namespace t {
constexpr int Wizard = 84;
constexpr int Bat = 120, Ghost = 121, Spider = 122, Rat = 123;
constexpr int Slime = 108, Cyclops = 109, Demon = 110;
constexpr int Sand = 48, SandSpeckleA = 49, SandSpeckleB = 51, SandRocks = 42;
constexpr int GraveCross = 64, GraveStone = 65, Barrel = 82, Crystal = 56;
constexpr int Staff = 129, Sword = 104, Shield = 102, Dagger = 103, Axe = 118;
constexpr int PotionGreen = 114, PotionRed = 115, PotionBlue = 116, Hammer = 117, Coin = 101;
constexpr int Chest = 89;
} // namespace t

} // namespace atlas
