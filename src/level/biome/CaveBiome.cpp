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
};
static_assert(std::size(CAVES) == int(CaveBiome::Count));

CaveBiome caveBelow(BiomeId above) { return BELOW[above]; }
