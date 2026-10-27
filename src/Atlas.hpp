#pragma once

struct Sprite {
    float x, y, w, h;
};

namespace atlas {
constexpr Sprite tile(int index) {
    return {float((index % 12) * 18 + 1), float((index / 12) * 18 + 1), 16, 16};
}
constexpr Sprite town(int index) {
    Sprite s = tile(index);
    s.x += 12 * 18;
    return s;
}
constexpr Sprite winter(int index) {
    Sprite s = town(index);
    s.y += 264;
    return s;
}
constexpr Sprite flora(int index) { return {float(216 + (index % 12) * 18 + 1), float(390 + (index / 12) * 18 + 1), 16, 16}; }
namespace fl {
constexpr int PinkLow = 0, PinkHigh = 1, WhiteLow = 2, WhiteHigh = 3, Lilac = 4, Berries = 5, Straw = 6;
constexpr int DeadPine = 7, SpiritPine = 8, MossPine = 9, BlueShrooms = 10, Petals = 11, PetalTufts = 12;
constexpr int WillowLow = 13, WillowHigh = 14, Reeds = 15, Toadstools = 16;
constexpr int Sand = 17, SandSpeck = 18, Cactus = 19, PalmLow = 20, PalmHigh = 21, SandPebbles = 22, BarrelCactus = 23;
} // namespace fl
constexpr Sprite mudRim(int offset) { return {float(216 + (offset % 12) * 18 + 1), float(426 + (offset / 12) * 18 + 1), 16, 16}; }
constexpr Sprite sandRim(int offset) { return {float(216 + (3 + offset % 12) * 18 + 1), float(426 + (offset / 12) * 18 + 1), 16, 16}; }
constexpr Sprite waterRim(int offset) { return {float(216 + (6 + offset % 12) * 18 + 1), float(426 + (offset / 12) * 18 + 1), 16, 16}; }
constexpr Sprite snowRim(int offset) { return {float(216 + (offset % 12) * 18 + 1), float(336 + (offset / 12) * 18 + 1), 16, 16}; }

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
constexpr int AutumnCrown = 3, Crown = 4, AutumnTrunk = 15, Trunk = 16;
constexpr int Bush = 5, Sprouts = 17, AutumnPine = 27, Pine = 28, Mushrooms = 29;
constexpr int DirtTopLeft = 12; // 3x3 grass-to-dirt edge set, centre at DirtTopLeft + 13
constexpr int Sign = 83, Beehive = 94, Log = 106, Sack = 107;
constexpr int Bomb = 105, Key = 117, Bow = 118, Arrow = 119, Scythe = 129;
constexpr int WellRoof = 92, Well = 104; // roof stacks on the well
constexpr int Path = 43;
constexpr int GreyRoof = 48, RedRoof = 52, RoofChimney = 3, RoofEaves = 12, RoofGable = 15;
constexpr int WoodWindow = 84, WoodDoor = 85, StoneWindow = 88, StoneDoor = 89;
constexpr int WoodWallLeft = 72, WoodWall = 73, WoodWallRight = 75, StoneWallLeft = 76, StoneWall = 77, StoneWallRight = 79;
constexpr int FenceTopLeft = 44, FenceTop = 45, FenceTopRight = 46, FenceLeft = 56, FenceRight = 59;
constexpr int FenceBottomLeft = 68, FenceBottomRight = 70, FenceEndLeft = 80, FenceRail = 81, FenceEndRight = 82;
constexpr int PlazaTopLeft = 96; // 3x3 raised stone slab, centre at PlazaTopLeft + 13
constexpr int ArchLeft = 113, ArchRight = 114, Bricks = 126;
constexpr int TowerTop = 96, WallTop = 99, Portcullis = 111, OpenGate = 113, ArrowSlit = 125;
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
constexpr int Chest = 89, Pillar = 58, Crate = 63;
} // namespace t

} // namespace atlas
