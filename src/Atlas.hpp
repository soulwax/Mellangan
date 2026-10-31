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
constexpr Sprite flora(int index) {
  return {float(216 + (index % 12) * 18 + 1),
          float(390 + (index / 12) * 18 + 1), 16, 16};
}
namespace fl {
constexpr int PinkLow = 0, PinkHigh = 1, WhiteLow = 2, WhiteHigh = 3, Lilac = 4,
              Berries = 5, Straw = 6;
constexpr int DeadPine = 7, SpiritPine = 8, MossPine = 9, BlueShrooms = 10,
              Petals = 11, PetalTufts = 12;
constexpr int WillowLow = 13, WillowHigh = 14, Reeds = 15, Toadstools = 16;
constexpr int Sand = 17, SandSpeck = 18, Cactus = 19, PalmLow = 20,
              PalmHigh = 21, SandPebbles = 22, BarrelCactus = 23;
} // namespace fl
constexpr Sprite mudRim(int offset) {
  return {float(216 + (offset % 12) * 18 + 1),
          float(426 + (offset / 12) * 18 + 1), 16, 16};
}
constexpr Sprite sandRim(int offset) {
  return {float(216 + (3 + offset % 12) * 18 + 1),
          float(426 + (offset / 12) * 18 + 1), 16, 16};
}
constexpr Sprite waterRim(int offset) {
  return {float(216 + (6 + offset % 12) * 18 + 1),
          float(426 + (offset / 12) * 18 + 1), 16, 16};
}
constexpr Sprite kit(int index) {
  return {float((index % 24) * 18 + 1), float(480 + (index / 24) * 18 + 1), 16,
          16};
}
namespace kt {
constexpr int BirchHigh = 0, BirchLow = 24;
constexpr int FrostBirchHigh = 1, FrostBirchLow = 25;
constexpr int CypressHigh = 2, CypressLow = 26;
constexpr int AcaciaHigh = 3, AcaciaLow = 27;
constexpr int BaobabHigh = 4, BaobabLow = 28;
constexpr int MushroomTreeHigh = 5, MushroomTreeLow = 29;
constexpr int JungleTreeHigh = 6, JungleTreeLow = 30;
constexpr int CharredTreeHigh = 7, CharredTreeLow = 31;
constexpr int CrystalSpireHigh = 8, CrystalSpireLow = 32;
constexpr int FirHigh = 9, FirLow = 33;
constexpr int SnowyFirHigh = 10, SnowyFirLow = 34;
constexpr int Fern = 11;
constexpr int FrostedFern = 12;
constexpr int TallGrass = 13;
constexpr int DryGrass = 14;
constexpr int Heather = 15;
constexpr int Thornbush = 16;
constexpr int Sunflower = 17;
constexpr int Tulips = 18;
constexpr int Lavender = 19;
constexpr int Wheat = 20;
constexpr int Cattails = 21;
constexpr int Daisies = 22;
constexpr int Boulder = 23;
constexpr int MossyBoulder = 35;
constexpr int SnowyBoulder = 36;
constexpr int Sandstone = 37;
constexpr int IceChunk = 38;
constexpr int Obsidian = 39;
constexpr int Pebbles = 40;
constexpr int SandTuft = 41;
constexpr int FrostTuft = 42;
constexpr int AshTuft = 43;
constexpr int BogTuft = 44;
constexpr int BloomTuft = 45;
constexpr int AppleTreeHigh = 48, AppleTreeLow = 72;
constexpr int AutumnBirchHigh = 49, AutumnBirchLow = 73;
constexpr int DeadOakHigh = 50, DeadOakLow = 74;
constexpr int WeepingWillowHigh = 51, WeepingWillowLow = 75;
constexpr int BambooHigh = 52, BambooLow = 76;
constexpr int RedwoodHigh = 53, RedwoodLow = 77;
constexpr int TreeFernHigh = 54, TreeFernLow = 78;
constexpr int BlueSpruceHigh = 55, BlueSpruceLow = 79;
constexpr int JoshuaTreeHigh = 56, JoshuaTreeLow = 80;
constexpr int Roses = 57;
constexpr int Blueberries = 58;
constexpr int Hydrangea = 59;
constexpr int Holly = 60;
constexpr int Pumpkins = 61;
constexpr int BrownMushrooms = 62;
constexpr int Stump = 63;
constexpr int MossyLog = 64;
constexpr int Poppies = 65;
constexpr int Cornflowers = 66;
constexpr int Bluebells = 67;
constexpr int Bracken = 68;
constexpr int Clover = 69;
constexpr int Menhir = 70;
constexpr int RockPile = 71;
constexpr int Amethyst = 81;
constexpr int Skull = 82;
constexpr int Driftwood = 83;
constexpr int PricklyPear = 84;
constexpr int Agave = 85;
constexpr int Tumbleweed = 86;
constexpr int Snowdrops = 87;
constexpr int FrozenShrub = 88;
constexpr int Glowshrooms = 89;
constexpr int Ivy = 90;
constexpr int Sage = 91;
constexpr int CallaLilies = 92;
constexpr int Strawberries = 93;
constexpr int Pampas = 94;
constexpr int StoneRing = 95;
} // namespace kt
constexpr Sprite carpet(int set, int cell) {
  return {float(((set % 2) * 12 + cell) * 18 + 1),
          float(552 + (set / 2) * 18 + 1), 16, 16};
}
namespace cp {
constexpr int Straw = 0, Ash = 1, CrystalMoss = 2, Mycelium = 3, Lava = 4,
              RedClay = 5, Needles = 6;
} // namespace cp
constexpr Sprite frontier(int index) {
  return {float((index % 12) * 18 + 1), float(270 + (index / 12) * 18 + 1), 16,
          16};
}
namespace fr {
constexpr int MesaHigh = 0, MesaLow = 12;
constexpr int HoodooHigh = 1, HoodooLow = 13;
constexpr int JuniperHigh = 2, JuniperLow = 14;
constexpr int TaigaSpruceHigh = 3, TaigaSpruceLow = 15;
constexpr int LarchHigh = 4, LarchLow = 16;
constexpr int RuneStoneHigh = 5, RuneStoneLow = 17;
constexpr int MineLeftHigh = 6, MineLeftLow = 18;
constexpr int MineMouthHigh = 7, MineMouthLow = 19;
constexpr int MineRightHigh = 8, MineRightLow = 20;
constexpr int Ocotillo = 9, RedRocks = 10, Ribs = 11, ClayTuft = 21;
constexpr int Lingonberry = 22, LichenRock = 23, CottonGrass = 24,
              NeedleTuft = 25, ShelfStump = 26;
constexpr int ToppledStone = 27, Altar = 28, MineCart = 29, OrePile = 30,
              LanternPost = 31, Timber = 32, RailAcross = 33, RailDown = 34;
constexpr int Wolf = 35, Alpha = 36, Burrower = 37, Mound = 38,
              Necromancer = 39, Skeleton = 40, Thief = 41, Chakram = 42,
              Glaive = 43, Censer = 44;
} // namespace fr
constexpr Sprite snowRim(int offset) {
  return {float(216 + (offset % 12) * 18 + 1),
          float(336 + (offset / 12) * 18 + 1), 16, 16};
}

constexpr Sprite expansion(int index) {
  return {float((index % 24) * 18 + 1), float(624 + (index / 24) * 18 + 1), 16,
          16};
}
constexpr Sprite arsenal(int index) {
  return {float((index % 24) * 18 + 1), float(1020 + (index / 24) * 18 + 1), 16,
          16};
}
namespace ex {
constexpr int MireLeech = 0, RicochetSling = 14;
constexpr int ShrineFury = 30, ShrineRoots = 31, ShrineVision = 32,
              ShrineFlame = 33;
} // namespace ex
constexpr Sprite biomeProp(int index) {
  return {float((index % 24) * 18 + 1), float(660 + (index / 24) * 18 + 1), 16,
          16};
}
constexpr Sprite dirtEdge(int set, int offset) {
  int i = set * 9 + offset / 12 * 3 + offset % 12;
  return {float((i % 24) * 18 + 1), float(786 + (i / 24) * 18 + 1), 16, 16};
}
constexpr Sprite waterEdge(int set, int offset) {
  int i = 144 + set * 9 + offset / 12 * 3 + offset % 12;
  return {float((i % 24) * 18 + 1), float(786 + (i / 24) * 18 + 1), 16, 16};
}
constexpr Sprite cliff(int offset) {
  int i = 297 + offset / 12 * 3 + offset % 12;
  return {float((i % 24) * 18 + 1), float(786 + (i / 24) * 18 + 1), 16, 16};
}
constexpr Sprite landmark(int index, int row, int col) {
  int i = index * 12 + row * 3 + col;
  return {float((i % 24) * 18 + 1), float(1236 + (i / 24.0f) * 18 + 1), 16, 16};
}
constexpr Sprite biomeGround(int set, int cell) {
  return {float(((set % 2) * 12 + cell) * 18 + 1),
          float(696 + (set / 2) * 18 + 1), 16, 16};
}

constexpr Sprite Glow{0, 200, 64, 64};
constexpr Sprite Light{256, 200, 64, 64};
constexpr Sprite Ring{64, 200, 64, 64};
constexpr Sprite Slash{128, 200, 24, 48};
constexpr Sprite Shadow{176, 200, 16, 8};
constexpr Sprite Gems[3] = {
    {197, 201, 7, 7}, {209, 201, 7, 7}, {221, 201, 7, 7}};
constexpr Sprite Bolt{232, 200, 10, 10};
constexpr Sprite Dot{244, 200, 8, 8};
constexpr Sprite White{197, 213, 2, 2};
constexpr Sprite digit(int d) { return {float(176 + d * 5), 220, 5, 7}; }

namespace pt {
constexpr int IceShard = 32, Snowflake = 33, FrostGlint = 34, Rock = 35,
              Pebble = 36, MeteorCore = 37, Ember = 38, ArcaneStar = 39,
              Wisp = 40, Rune = 41, HolyGlint = 42, Coin = 43, Bubble = 44,
              Petal = 45, Leaf = 46, Feather = 47, Puff = 48, Crescent = 49,
              Bone = 50, MapleLeaf = 51, Rain = 52, Gust = 53, Fluff = 54,
              FallingStar = 55, Butterfly = 56, Spore = 57;
} // namespace pt

namespace wx {
constexpr Sprite Cloud{0, 0, 96, 96}, FogA{96, 0, 64, 64}, FogB{160, 0, 64, 64},
    Drop{64, 96, 1, 6};
constexpr Sprite ripple(int f) { return {float(f * 8), 96, 8, 8}; }
constexpr Sprite splash(int f) { return {float(32 + f * 8), 96, 8, 8}; }
constexpr Sprite flake(int i) { return {float(48 + i * 4), 96, 4, 4}; }
constexpr Sprite ash(int i) { return {float(72 + i * 3), 96, 3, 3}; }
constexpr Sprite puddle(int i) {
  return i ? Sprite{16, 104, 12, 6} : Sprite{0, 104, 16, 8};
}
constexpr Sprite drift(int i) {
  return i ? Sprite{48, 104, 12, 6} : Sprite{32, 104, 16, 8};
}
} // namespace wx

namespace tt {
constexpr int Grass = 0, GrassTufts = 1, GrassFlowers = 2;
constexpr int AutumnCrown = 3, Crown = 4, AutumnTrunk = 15, Trunk = 16;
constexpr int Bush = 5, Sprouts = 17, AutumnPine = 27, Pine = 28,
              Mushrooms = 29;
constexpr int DirtTopLeft =
    12; // 3x3 grass-to-dirt edge set, centre at DirtTopLeft + 13
constexpr int Sign = 83, Beehive = 94, Log = 106, Sack = 107;
constexpr int Bomb = 105, Key = 117, Bow = 118, Arrow = 119, Scythe = 129;
constexpr int WellRoof = 92, Well = 104; // roof stacks on the well
constexpr int Path = 43;
constexpr int GreyRoof = 48, RedRoof = 52, RoofChimney = 3, RoofEaves = 12,
              RoofGable = 15;
constexpr int WoodWindow = 84, WoodDoor = 85, StoneWindow = 88, StoneDoor = 89;
constexpr int WoodWallLeft = 72, WoodWall = 73, WoodWallRight = 75,
              StoneWallLeft = 76, StoneWall = 77, StoneWallRight = 79;
constexpr int FenceTopLeft = 44, FenceTop = 45, FenceTopRight = 46,
              FenceLeft = 56, FenceRight = 59;
constexpr int FenceBottomLeft = 68, FenceBottomRight = 70, FenceEndLeft = 80,
              FenceRail = 81, FenceEndRight = 82;
constexpr int PlazaTopLeft =
    96; // 3x3 raised stone slab, centre at PlazaTopLeft + 13
constexpr int ArchLeft = 113, ArchRight = 114, Bricks = 126;
constexpr int TowerTop = 96, WallTop = 99, Portcullis = 111, OpenGate = 113,
              ArrowSlit = 125;
} // namespace tt

namespace t {
constexpr int Wizard = 84;
constexpr int Bat = 120, Ghost = 121, Spider = 122, Rat = 123, Scorpion = 124;
constexpr int Slime = 108, Cyclops = 109, Demon = 110, Cultist = 111,
              Knight = 96;
constexpr int Sand = 48, SandSpeckleA = 49, SandSpeckleB = 51, SandRocks = 42;
constexpr int GraveCross = 64, GraveStone = 65, Barrel = 82, Crystal = 56;
constexpr int Staff = 129, Sword = 104, Shield = 102, Dagger = 103, Axe = 118;
constexpr int PotionGreen = 114, PotionRed = 115, PotionBlue = 116,
              Hammer = 117, Coin = 101;
constexpr int PotionGrey = 113, VialGreen = 126, VialBlue = 128,
              StormStaff = 130;
constexpr int Chest = 89, Pillar = 58, Crate = 63;
} // namespace t

} // namespace atlas
