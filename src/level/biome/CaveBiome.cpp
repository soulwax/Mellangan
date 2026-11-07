#include "level/biome/CaveBiome.hpp"

#include <iterator>

namespace {

namespace f = feature;
namespace tt = atlas::tt;
namespace t = atlas::t;
namespace cp = atlas::cp;
using K = EnemyKind;

constexpr Sprite dirt = atlas::town(tt::DirtTopLeft + 13);

constexpr CaveBiome BELOW[BIOME_COUNT] = {
    CaveBiome::Roots,    CaveBiome::Roots,     CaveBiome::Roots,   CaveBiome::Roots,
    CaveBiome::Catacombs, CaveBiome::Rime,     CaveBiome::Roots,   CaveBiome::Grotto,
    CaveBiome::Warrens,  CaveBiome::Grotto,    CaveBiome::Warrens, CaveBiome::Vents,
    CaveBiome::Geodes,   CaveBiome::Spores,    CaveBiome::Warrens, CaveBiome::Rime,
    CaveBiome::Warrens,  CaveBiome::Grotto,    CaveBiome::Rime,    CaveBiome::Grotto,
    CaveBiome::Vents,    CaveBiome::Roots,     CaveBiome::Warrens, CaveBiome::Warrens,
    CaveBiome::Geodes,   CaveBiome::Vents};
static_assert(std::size(BELOW) == BIOME_COUNT);

struct DeepClimate {
  CaveBiome biome;
  float lo[3], hi[3];
};

constexpr float A = -9, Z = 9;
constexpr DeepClimate DEEP_CLIMATES[] = {
    {CaveBiome::Magma, {0.22f, A, A}, {Z, Z, Z}},     {CaveBiome::Cathedral, {A, A, A}, {-0.22f, Z, Z}},
    {CaveBiome::Mere, {A, 0.2f, A}, {Z, Z, Z}},       {CaveBiome::Hive, {0.04f, 0.02f, A}, {Z, Z, Z}},
    {CaveBiome::Glowcap, {A, 0.02f, A}, {Z, Z, Z}},   {CaveBiome::Halls, {A, A, 0.16f}, {Z, Z, Z}},
    {CaveBiome::Ossuary, {A, A, -0.08f}, {Z, Z, Z}},  {CaveBiome::Abyss, {A, A, A}, {Z, Z, Z}},
};

} // namespace

const CaveDef CAVES[int(CaveBiome::Count)] = {
    {"ROOT HOLLOWS",
     {dirt, dirt, dirt},
     0,
     {0.62f, 0.52f, 0.42f, 1},
     {0.5f, 0.4f, 0.32f, 1},
     {0.55f, 0.4f, 0.25f, 1},
     {{&f::mossyLog, 14}, {&f::brownMushrooms, 22}, {&f::glowshrooms, 6}},
     foe(K::Burrower) | foe(K::Spider) | foe(K::Rat) | foe(K::KoboldMiner) | foe(K::CaveBat),
     -0.42f,
     CavePool::Water},
    {"DRIPSTONE GROTTO",
     {atlas::biomeGround(3, 0), atlas::biomeGround(3, 1), atlas::biomeGround(3, 2)},
     1,
     {0.5f, 0.58f, 0.64f, 1},
     {0.62f, 0.7f, 0.76f, 1},
     {0.3f, 0.5f, 0.6f, 1},
     {{&f::lichenRock, 18}, {&f::pebbles, 26}, {&f::blueShrooms, 8}},
     foe(K::MireLeech) | foe(K::Drowned) | foe(K::Snapjaw) | foe(K::CaveBat) | foe(K::BogToad),
     -0.18f,
     CavePool::Water},
    {"RIME CAVES",
     {atlas::winter(tt::Grass), atlas::winter(tt::GrassTufts), atlas::winter(tt::Grass)},
     3,
     {0.7f, 0.78f, 0.9f, 1},
     {0.85f, 0.92f, 1, 1},
     {0.7f, 0.85f, 1, 1},
     {{&f::iceChunk, 18}, {&f::snowyBoulder, 14}, {&f::frozenShrub, 8}},
     foe(K::FrostWisp) | foe(K::Iceback) | foe(K::Yeti) | foe(K::IceWraith) | foe(K::CaveBat),
     -0.3f,
     CavePool::Ice},
    {"SANDSTONE WARRENS",
     {atlas::tile(t::Sand), atlas::tile(t::SandSpeckleA), atlas::tile(t::SandSpeckleB)},
     2,
     {0.72f, 0.62f, 0.48f, 1},
     {0.82f, 0.7f, 0.55f, 1},
     {0.85f, 0.7f, 0.45f, 1},
     {{&f::sandstone, 16}, {&f::ribs, 8}, {&f::skull, 8}},
     foe(K::SandWyrm) | foe(K::Burrower) | foe(K::Scorpion) | foe(K::GlassSpitter) | foe(K::RockCrawler),
     -0.6f,
     CavePool::Water},
    {"SPORE HALLS",
     {atlas::carpet(cp::Mycelium, 0), atlas::carpet(cp::Mycelium, 1), atlas::carpet(cp::Mycelium, 2)},
     5,
     {0.6f, 0.5f, 0.62f, 1},
     {0.62f, 0.6f, 0.55f, 1},
     {0.75f, 0.45f, 0.75f, 1},
     {{&f::glowshrooms, 22}, {&f::toadstools, 18}, {&f::mushrooms, 14}},
     foe(K::Sporeling) | foe(K::MycelialBrute) | foe(K::SporePuffer) | foe(K::CaveHorror),
     -0.5f,
     CavePool::Water},
    {"EMBER VENTS",
     {atlas::carpet(cp::Ash, 0), atlas::carpet(cp::Ash, 1), atlas::carpet(cp::Ash, 0)},
     4,
     {0.7f, 0.58f, 0.54f, 1},
     {0.7f, 0.58f, 0.54f, 1},
     {0.9f, 0.35f, 0.15f, 1},
     {{&f::obsidian, 16}, {&f::charredTree, 6}, {&f::ashTuft, 20}},
     foe(K::EmberImp) | foe(K::CinderGolem) | foe(K::MagmaSlug) | foe(K::AshWraith) | foe(K::FlameCultist),
     -0.2f,
     CavePool::Lava},
    {"CRYSTAL GEODES",
     {atlas::carpet(cp::CrystalMoss, 0), atlas::carpet(cp::CrystalMoss, 1), atlas::carpet(cp::CrystalMoss, 2)},
     1,
     {0.6f, 0.55f, 0.7f, 1},
     {0.72f, 0.62f, 0.9f, 1},
     {0.65f, 0.45f, 0.95f, 1},
     {{&f::crystal, 18}, {&f::amethyst, 16}, {&f::crystalSpire, 4}},
     foe(K::CrystalGolem) | foe(K::CrystalShard) | foe(K::GlassMantis) | foe(K::OreGolem) | foe(K::CaveBat),
     -0.5f,
     CavePool::Water},
    {"CATACOMBS",
     {atlas::town(tt::PlazaTopLeft + 13), atlas::town(tt::PlazaTopLeft + 13), atlas::town(tt::PlazaTopLeft + 13)},
     1,
     {0.56f, 0.55f, 0.6f, 1},
     {0.6f, 0.6f, 0.64f, 1},
     {0.55f, 0.6f, 0.5f, 1},
     {{&f::grave, 20}, {&f::skull, 14}, {&f::ribs, 8}},
     foe(K::Skeleton) | foe(K::BarrowWight) | foe(K::BarrowArcher) | foe(K::Necromancer) | foe(K::Ghost),
     -0.7f,
     CavePool::Water},
    {"GLOWCAP FOREST",
     {atlas::carpet(cp::Mycelium, 0), atlas::carpet(cp::Mycelium, 1), atlas::carpet(cp::Mycelium, 2)},
     5,
     {0.42f, 0.56f, 0.78f, 1},
     {0.4f, 0.5f, 0.7f, 1},
     {0.35f, 0.6f, 0.95f, 1},
     {{&f::glowshrooms, 34}, {&f::blueShrooms, 26}, {&f::mushroomTree, 5}},
     foe(K::Sporeling) | foe(K::SporePuffer) | foe(K::MothWisp) | foe(K::Spider) | foe(K::CaveBat),
     -0.45f,
     CavePool::Water},
    {"BLACK MERE",
     {atlas::biomeGround(3, 0), atlas::biomeGround(3, 1), atlas::biomeGround(3, 2)},
     1,
     {0.36f, 0.46f, 0.5f, 1},
     {0.42f, 0.5f, 0.56f, 1},
     {0.2f, 0.32f, 0.42f, 1},
     {{&f::lichenRock, 14}, {&f::pebbles, 22}, {&f::cattails, 12}},
     foe(K::Drowned) | foe(K::MireLeech) | foe(K::Snapjaw) | foe(K::BogToad) | foe(K::SunkenPriest),
     0.05f,
     CavePool::Water},
    {"MAGMA HALLS",
     {atlas::carpet(cp::Ash, 0), atlas::carpet(cp::Ash, 1), atlas::carpet(cp::Ash, 0)},
     4,
     {0.66f, 0.5f, 0.44f, 1},
     {0.62f, 0.46f, 0.4f, 1},
     {1, 0.3f, 0.1f, 1},
     {{&f::obsidian, 22}, {&f::ashTuft, 16}, {&f::charredTree, 4}},
     foe(K::CinderGolem) | foe(K::EmberImp) | foe(K::MagmaSlug) | foe(K::FlameCultist) | foe(K::EmberDrake),
     -0.08f,
     CavePool::Lava},
    {"CRYSTAL CATHEDRAL",
     {atlas::carpet(cp::CrystalMoss, 0), atlas::carpet(cp::CrystalMoss, 1), atlas::carpet(cp::CrystalMoss, 2)},
     3,
     {0.66f, 0.7f, 0.9f, 1},
     {0.8f, 0.85f, 1, 1},
     {0.8f, 0.85f, 1, 1},
     {{&f::crystalSpire, 10}, {&f::crystal, 22}, {&f::iceChunk, 10}},
     foe(K::CrystalGolem) | foe(K::CrystalShard) | foe(K::GlassMantis) | foe(K::IceWraith) | foe(K::FrostWisp),
     -0.3f,
     CavePool::Ice},
    {"OSSUARY PLAINS",
     {atlas::tile(t::Sand), atlas::tile(t::SandSpeckleA), atlas::tile(t::SandSpeckleB)},
     2,
     {0.76f, 0.74f, 0.68f, 1},
     {0.74f, 0.7f, 0.62f, 1},
     {0.85f, 0.82f, 0.7f, 1},
     {{&f::ribs, 20}, {&f::skull, 20}, {&f::rockPile, 8}},
     foe(K::Skeleton) | foe(K::BarrowArcher) | foe(K::BarrowWight) | foe(K::Necromancer) | foe(K::GraveRobber),
     -0.7f,
     CavePool::Water},
    {"LOST HALLS",
     {atlas::town(tt::PlazaTopLeft + 13), atlas::town(tt::PlazaTopLeft + 13), atlas::town(tt::PlazaTopLeft + 13)},
     1,
     {0.64f, 0.6f, 0.56f, 1},
     {0.66f, 0.62f, 0.58f, 1},
     {0.85f, 0.7f, 0.4f, 1},
     {{&f::runeStone, 6}, {&f::grave, 10}, {&f::menhir, 6}},
     foe(K::Ghost) | foe(K::Shade) | foe(K::HollowSentinel) | foe(K::BarrowWight) | foe(K::Skeleton),
     -0.6f,
     CavePool::Water},
    {"THE CHASM",
     {atlas::town(tt::PlazaTopLeft + 13), atlas::town(tt::PlazaTopLeft + 13), atlas::town(tt::PlazaTopLeft + 13)},
     0,
     {0.4f, 0.38f, 0.46f, 1},
     {0.42f, 0.4f, 0.5f, 1},
     {0.4f, 0.35f, 0.55f, 1},
     {{&f::rockPile, 12}, {&f::boulder, 10}, {&f::skull, 4}},
     foe(K::CaveHorror) | foe(K::StarSpawn) | foe(K::Starlet) | foe(K::NightStalker) | foe(K::AshWraith),
     -0.8f,
     CavePool::Water},
    {"CHITIN HIVE",
     {atlas::carpet(cp::RedClay, 0), atlas::carpet(cp::RedClay, 1), atlas::carpet(cp::RedClay, 2)},
     2,
     {0.8f, 0.64f, 0.42f, 1},
     {0.7f, 0.56f, 0.36f, 1},
     {0.9f, 0.65f, 0.25f, 1},
     {{&f::beehive, 10}, {&f::shelfStump, 8}, {&f::mushrooms, 8}},
     foe(K::Spider) | foe(K::Scorpion) | foe(K::AmberBeetle) | foe(K::RockCrawler) | foe(K::Burrower),
     -0.4f,
     CavePool::Water},
};
static_assert(std::size(CAVES) == int(CaveBiome::Count));

CaveBiome caveBelow(BiomeId above) { return BELOW[above]; }

CaveBiome deepClimate(float heat, float damp, float age) {
  float v[3] = {heat, damp, age};
  for (const DeepClimate &c : DEEP_CLIMATES)
    if (v[0] >= c.lo[0] && v[0] < c.hi[0] && v[1] >= c.lo[1] && v[1] < c.hi[1] && v[2] >= c.lo[2] && v[2] < c.hi[2])
      return c.biome;
  return CaveBiome::Abyss;
}
