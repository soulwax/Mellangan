#include "level/Feature.hpp"

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;

constexpr Color NONE{0, 0, 0, 0};

Sprite sheet(const Feature *f, int index) {
    if (f->art) return f->art(index);
    return f->winter ? atlas::winter(index) : f->town ? atlas::town(index) : atlas::tile(index);
}

} // namespace

bool placeStack(const Feature *f, uint32_t, Prop *p) {
    p->add(sheet(f, f->tile));
    if (f->top >= 0) p->add(sheet(f, f->top), -16);
    p->shadow = f->shadow, p->solid = f->solid, p->light = f->light;
    return true;
}

bool placeGrave(const Feature *f, uint32_t roll, Prop *p) {
    p->add(sheet(f, roll % 10 < 3 ? t::GraveCross : f->tile));
    p->shadow = f->shadow, p->solid = f->solid;
    if (roll % 3 == 0) p->light = f->light;
    return true;
}

namespace feature {

const Feature oak{"oak", placeStack, tt::Trunk, tt::Crown, true, 1.3f, 3, NONE};
const Feature autumnOak{"autumn oak", placeStack, tt::AutumnTrunk, tt::AutumnCrown, true, 1.3f, 3, NONE};
const Feature pine{"pine", placeStack, tt::Pine, -1, true, 1.0f, 3, NONE};
const Feature autumnPine{"autumn pine", placeStack, tt::AutumnPine, -1, true, 1.0f, 3, NONE};
const Feature bush{"bush", placeStack, tt::Bush, -1, true, 0.9f, 4, NONE};
const Feature snowOak{"snowy oak", placeStack, tt::Trunk, tt::Crown, true, 1.3f, 3, NONE, true};
const Feature snowPine{"snowy pine", placeStack, tt::Pine, -1, true, 1.0f, 3, NONE, true};
const Feature snowBush{"snowy bush", placeStack, tt::Bush, -1, true, 0.9f, 4, NONE, true};

const Feature pinkOak{"pink blossom", placeStack, atlas::fl::PinkLow, atlas::fl::PinkHigh, true, 1.3f, 3, NONE, false, atlas::flora};
const Feature whiteOak{"white blossom", placeStack, atlas::fl::WhiteLow, atlas::fl::WhiteHigh, true, 1.3f, 3, NONE, false, atlas::flora};
const Feature lilac{"lilac", placeStack, atlas::fl::Lilac, -1, true, 0.9f, 4, NONE, false, atlas::flora};
const Feature berryBush{"berry bush", placeStack, atlas::fl::Berries, -1, true, 0.9f, 4, NONE, false, atlas::flora};
const Feature dryBush{"dry bush", placeStack, atlas::fl::Straw, -1, true, 0.9f, 4, NONE, false, atlas::flora};
const Feature deadPine{"dead pine", placeStack, atlas::fl::DeadPine, -1, true, 1.0f, 3, NONE, false, atlas::flora};
const Feature spiritPine{"spirit pine", placeStack, atlas::fl::SpiritPine, -1, true, 1.0f, 3, {0.04f, 0.16f, 0.17f, 1}, false, atlas::flora};
const Feature mossPine{"moss pine", placeStack, atlas::fl::MossPine, -1, true, 1.0f, 3, NONE, false, atlas::flora};
const Feature cactus{"cactus", placeStack, atlas::fl::Cactus, -1, true, 0.8f, 3, NONE, false, atlas::flora};
const Feature barrelCactus{"barrel cactus", placeStack, atlas::fl::BarrelCactus, -1, true, 0.8f, 4, NONE, false, atlas::flora};
const Feature palm{"palm", placeStack, atlas::fl::PalmLow, atlas::fl::PalmHigh, true, 1.3f, 3, NONE, false, atlas::flora};
const Feature birch{"birch", placeStack, atlas::kt::BirchLow, atlas::kt::BirchHigh, true, 1.3f, 3, NONE, false, atlas::kit};
const Feature frostBirch{"frostBirch", placeStack, atlas::kt::FrostBirchLow, atlas::kt::FrostBirchHigh, true, 1.3f, 3, NONE, false, atlas::kit};
const Feature cypress{"cypress", placeStack, atlas::kt::CypressLow, atlas::kt::CypressHigh, true, 1.1f, 3, NONE, false, atlas::kit};
const Feature acacia{"acacia", placeStack, atlas::kt::AcaciaLow, atlas::kt::AcaciaHigh, true, 1.5f, 3, NONE, false, atlas::kit};
const Feature baobab{"baobab", placeStack, atlas::kt::BaobabLow, atlas::kt::BaobabHigh, true, 1.6f, 5, NONE, false, atlas::kit};
const Feature mushroomTree{"mushroomTree", placeStack, atlas::kt::MushroomTreeLow, atlas::kt::MushroomTreeHigh, true, 1.4f, 4, {0.16f, 0.04f, 0.05f, 1}, false, atlas::kit};
const Feature jungleTree{"jungleTree", placeStack, atlas::kt::JungleTreeLow, atlas::kt::JungleTreeHigh, true, 1.3f, 3, NONE, false, atlas::kit};
const Feature charredTree{"charredTree", placeStack, atlas::kt::CharredTreeLow, atlas::kt::CharredTreeHigh, true, 1.0f, 3, {0.22f, 0.08f, 0.02f, 1}, false, atlas::kit};
const Feature crystalSpire{"crystalSpire", placeStack, atlas::kt::CrystalSpireLow, atlas::kt::CrystalSpireHigh, true, 1.2f, 5, {0.10f, 0.08f, 0.26f, 1}, false, atlas::kit};
const Feature fir{"fir", placeStack, atlas::kt::FirLow, atlas::kt::FirHigh, true, 1.4f, 3, NONE, false, atlas::kit};
const Feature snowyFir{"snowyFir", placeStack, atlas::kt::SnowyFirLow, atlas::kt::SnowyFirHigh, true, 1.4f, 3, NONE, false, atlas::kit};
const Feature fern{"fern", placeStack, atlas::kt::Fern, -1, true, 0.4f, 0, NONE, false, atlas::kit};
const Feature frostedFern{"frostedFern", placeStack, atlas::kt::FrostedFern, -1, true, 0.4f, 0, NONE, false, atlas::kit};
const Feature tallGrass{"tallGrass", placeStack, atlas::kt::TallGrass, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature dryGrass{"dryGrass", placeStack, atlas::kt::DryGrass, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature heather{"heather", placeStack, atlas::kt::Heather, -1, true, 0.8f, 4, NONE, false, atlas::kit};
const Feature thornbush{"thornbush", placeStack, atlas::kt::Thornbush, -1, true, 0.7f, 4, NONE, false, atlas::kit};
const Feature sunflower{"sunflower", placeStack, atlas::kt::Sunflower, -1, true, 0.4f, 0, NONE, false, atlas::kit};
const Feature tulips{"tulips", placeStack, atlas::kt::Tulips, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature lavender{"lavender", placeStack, atlas::kt::Lavender, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature wheat{"wheat", placeStack, atlas::kt::Wheat, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature cattails{"cattails", placeStack, atlas::kt::Cattails, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature daisies{"daisies", placeStack, atlas::kt::Daisies, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature boulder{"boulder", placeStack, atlas::kt::Boulder, -1, true, 1.0f, 6, NONE, false, atlas::kit};
const Feature mossyBoulder{"mossyBoulder", placeStack, atlas::kt::MossyBoulder, -1, true, 1.0f, 6, NONE, false, atlas::kit};
const Feature snowyBoulder{"snowyBoulder", placeStack, atlas::kt::SnowyBoulder, -1, true, 1.0f, 6, NONE, false, atlas::kit};
const Feature sandstone{"sandstone", placeStack, atlas::kt::Sandstone, -1, true, 0.9f, 6, NONE, false, atlas::kit};
const Feature iceChunk{"iceChunk", placeStack, atlas::kt::IceChunk, -1, true, 0.8f, 5, {0.05f, 0.12f, 0.18f, 1}, false, atlas::kit};
const Feature obsidian{"obsidian", placeStack, atlas::kt::Obsidian, -1, true, 0.8f, 5, NONE, false, atlas::kit};
const Feature pebbles{"pebbles", placeStack, atlas::kt::Pebbles, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature sandTuft{"sandTuft", placeStack, atlas::kt::SandTuft, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature frostTuft{"frostTuft", placeStack, atlas::kt::FrostTuft, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature ashTuft{"ashTuft", placeStack, atlas::kt::AshTuft, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature bogTuft{"bogTuft", placeStack, atlas::kt::BogTuft, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature bloomTuft{"bloomTuft", placeStack, atlas::kt::BloomTuft, -1, true, 0, 0, NONE, false, atlas::kit};
const Feature willow{"willow", placeStack, atlas::fl::WillowLow, atlas::fl::WillowHigh, true, 1.3f, 3, NONE, false, atlas::flora};
const Feature reeds{"reeds", placeStack, atlas::fl::Reeds, -1, true, 0.4f, 0, NONE, false, atlas::flora};
const Feature toadstools{"toadstools", placeStack, atlas::fl::Toadstools, -1, true, 0, 0, {0.12f, 0.04f, 0.2f, 1}, false, atlas::flora};
const Feature blueShrooms{"blue mushrooms", placeStack, atlas::fl::BlueShrooms, -1, true, 0, 0, {0.04f, 0.07f, 0.22f, 1}, false, atlas::flora};

const Feature grave{"grave", placeGrave, t::GraveStone, -1, false, 0, 4, {0.07f, 0.15f, 0.32f, 1}};
const Feature mushrooms{"mushrooms", placeStack, tt::Mushrooms, -1, true, 0, 0, {0.03f, 0.12f, 0.10f, 1}};
const Feature sprouts{"sprouts", placeStack, tt::Sprouts, -1, true, 0, 0, NONE};
const Feature log{"log", placeStack, tt::Log, -1, true, 0.9f, 5, NONE};
const Feature beehive{"beehive", placeStack, tt::Beehive, -1, true, 0.8f, 4, NONE};
const Feature sign{"sign", placeStack, tt::Sign, -1, true, 0.6f, 3, NONE};
const Feature sack{"sack", placeStack, tt::Sack, -1, true, 0.6f, 4, NONE};
const Feature crystal{"crystal", placeStack, t::Crystal, -1, false, 0.7f, 4, {0.05f, 0.18f, 0.30f, 1}};
const Feature well{"well", placeStack, tt::Well, tt::WellRoof, true, 1.0f, 6, NONE};
const Feature barrel{"barrel", placeStack, t::Barrel, -1, false, 0.8f, 4, NONE};
const Feature crate{"crate", placeStack, t::Crate, -1, false, 0.8f, 5, NONE};

} // namespace feature
