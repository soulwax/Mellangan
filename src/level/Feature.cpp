#include "level/Feature.hpp"

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;
namespace fl = atlas::fl;
namespace kt = atlas::kt;
namespace fr = atlas::fr;

constexpr Color NONE{0, 0, 0, 0};

Sprite sheet(const Feature *f, int index) {
  if (f->art)
    return f->art(index);
  return f->winter ? atlas::winter(index)
         : f->town ? atlas::town(index)
                   : atlas::tile(index);
}

constexpr Feature town(const char *name, int tile, int top, float shadow,
                       float solid, Color light = NONE) {
  return {name, placeStack, tile, top, true, shadow, solid, light};
}

constexpr Feature dungeon(const char *name, int tile, int top, float shadow,
                          float solid, Color light = NONE) {
  return {name, placeStack, tile, top, false, shadow, solid, light};
}

constexpr Feature snowy(const char *name, int tile, int top, float shadow,
                        float solid) {
  return {name, placeStack, tile, top, true, shadow, solid, NONE, true};
}

constexpr Feature drawn(Sprite (*art)(int), const char *name, int tile, int top,
                        float shadow, float solid, Color light) {
  return {name, placeStack, tile, top, true, shadow, solid, light, false, art};
}

constexpr Feature flora(const char *name, int tile, int top, float shadow,
                        float solid, Color light = NONE) {
  return drawn(atlas::flora, name, tile, top, shadow, solid, light);
}

constexpr Feature kit(const char *name, int tile, int top, float shadow,
                      float solid, Color light = NONE) {
  return drawn(atlas::kit, name, tile, top, shadow, solid, light);
}

constexpr Feature frontier(const char *name, int tile, int top, float shadow,
                           float solid, Color light = NONE) {
  return drawn(atlas::frontier, name, tile, top, shadow, solid, light);
}

} // namespace

bool placeStack(const Feature *f, uint32_t, Prop *p) {
  p->add(sheet(f, f->tile));
  if (f->top >= 0)
    p->add(sheet(f, f->top), -16);
  p->shadow = f->shadow, p->solid = f->solid, p->light = f->light;
  return true;
}

bool placeGrave(const Feature *f, uint32_t roll, Prop *p) {
  p->add(sheet(f, roll % 10 < 3 ? t::GraveCross : f->tile));
  p->shadow = f->shadow, p->solid = f->solid;
  if (roll % 3 == 0)
    p->light = f->light;
  return true;
}

namespace feature {

const Feature oak = town("oak", tt::Trunk, tt::Crown, 1.3f, 3);
const Feature autumnOak = town("autumn oak", tt::AutumnTrunk, tt::AutumnCrown, 1.3f, 3);
const Feature pine = town("pine", tt::Pine, -1, 1.0f, 3);
const Feature autumnPine = town("autumn pine", tt::AutumnPine, -1, 1.0f, 3);
const Feature bush = town("bush", tt::Bush, -1, 0.9f, 4);
const Feature snowOak = snowy("snowy oak", tt::Trunk, tt::Crown, 1.3f, 3);
const Feature snowPine = snowy("snowy pine", tt::Pine, -1, 1.0f, 3);
const Feature snowBush = snowy("snowy bush", tt::Bush, -1, 0.9f, 4);

const Feature pinkOak = flora("pink blossom", fl::PinkLow, fl::PinkHigh, 1.3f, 3);
const Feature whiteOak = flora("white blossom", fl::WhiteLow, fl::WhiteHigh, 1.3f, 3);
const Feature lilac = flora("lilac", fl::Lilac, -1, 0.9f, 4);
const Feature berryBush = flora("berry bush", fl::Berries, -1, 0.9f, 4);
const Feature dryBush = flora("dry bush", fl::Straw, -1, 0.9f, 4);
const Feature deadPine = flora("dead pine", fl::DeadPine, -1, 1.0f, 3);
const Feature spiritPine =
    flora("spirit pine", fl::SpiritPine, -1, 1.0f, 3, {0.04f, 0.16f, 0.17f, 1});
const Feature mossPine = flora("moss pine", fl::MossPine, -1, 1.0f, 3);
const Feature cactus = flora("cactus", fl::Cactus, -1, 0.8f, 3);
const Feature barrelCactus = flora("barrel cactus", fl::BarrelCactus, -1, 0.8f, 4);
const Feature palm = flora("palm", fl::PalmLow, fl::PalmHigh, 1.3f, 3);
const Feature willow = flora("willow", fl::WillowLow, fl::WillowHigh, 1.3f, 3);
const Feature reeds = flora("reeds", fl::Reeds, -1, 0.4f, 0);
const Feature toadstools =
    flora("toadstools", fl::Toadstools, -1, 0, 0, {0.12f, 0.04f, 0.2f, 1});
const Feature blueShrooms =
    flora("blue mushrooms", fl::BlueShrooms, -1, 0, 0, {0.04f, 0.07f, 0.22f, 1});

const Feature birch = kit("birch", kt::BirchLow, kt::BirchHigh, 1.3f, 3);
const Feature frostBirch = kit("frostBirch", kt::FrostBirchLow, kt::FrostBirchHigh, 1.3f, 3);
const Feature cypress = kit("cypress", kt::CypressLow, kt::CypressHigh, 1.1f, 3);
const Feature acacia = kit("acacia", kt::AcaciaLow, kt::AcaciaHigh, 1.5f, 3);
const Feature baobab = kit("baobab", kt::BaobabLow, kt::BaobabHigh, 1.6f, 5);
const Feature mushroomTree = kit("mushroomTree", kt::MushroomTreeLow, kt::MushroomTreeHigh,
                                 1.4f, 4, {0.16f, 0.04f, 0.05f, 1});
const Feature jungleTree = kit("jungleTree", kt::JungleTreeLow, kt::JungleTreeHigh, 1.3f, 3);
const Feature charredTree = kit("charredTree", kt::CharredTreeLow, kt::CharredTreeHigh,
                                1.0f, 3, {0.22f, 0.08f, 0.02f, 1});
const Feature crystalSpire = kit("crystalSpire", kt::CrystalSpireLow, kt::CrystalSpireHigh,
                                 1.2f, 5, {0.10f, 0.08f, 0.26f, 1});
const Feature fir = kit("fir", kt::FirLow, kt::FirHigh, 1.4f, 3);
const Feature snowyFir = kit("snowyFir", kt::SnowyFirLow, kt::SnowyFirHigh, 1.4f, 3);
const Feature fern = kit("fern", kt::Fern, -1, 0.4f, 0);
const Feature frostedFern = kit("frostedFern", kt::FrostedFern, -1, 0.4f, 0);
const Feature tallGrass = kit("tallGrass", kt::TallGrass, -1, 0, 0);
const Feature dryGrass = kit("dryGrass", kt::DryGrass, -1, 0, 0);
const Feature heather = kit("heather", kt::Heather, -1, 0.8f, 4);
const Feature thornbush = kit("thornbush", kt::Thornbush, -1, 0.7f, 4);
const Feature sunflower = kit("sunflower", kt::Sunflower, -1, 0.4f, 0);
const Feature tulips = kit("tulips", kt::Tulips, -1, 0, 0);
const Feature lavender = kit("lavender", kt::Lavender, -1, 0, 0);
const Feature wheat = kit("wheat", kt::Wheat, -1, 0, 0);
const Feature cattails = kit("cattails", kt::Cattails, -1, 0, 0);
const Feature daisies = kit("daisies", kt::Daisies, -1, 0, 0);
const Feature boulder = kit("boulder", kt::Boulder, -1, 1.0f, 6);
const Feature mossyBoulder = kit("mossyBoulder", kt::MossyBoulder, -1, 1.0f, 6);
const Feature snowyBoulder = kit("snowyBoulder", kt::SnowyBoulder, -1, 1.0f, 6);
const Feature sandstone = kit("sandstone", kt::Sandstone, -1, 0.9f, 6);
const Feature iceChunk = kit("iceChunk", kt::IceChunk, -1, 0.8f, 5, {0.05f, 0.12f, 0.18f, 1});
const Feature obsidian = kit("obsidian", kt::Obsidian, -1, 0.8f, 5);
const Feature pebbles = kit("pebbles", kt::Pebbles, -1, 0, 0);
const Feature sandTuft = kit("sandTuft", kt::SandTuft, -1, 0, 0);
const Feature frostTuft = kit("frostTuft", kt::FrostTuft, -1, 0, 0);
const Feature ashTuft = kit("ashTuft", kt::AshTuft, -1, 0, 0);
const Feature bogTuft = kit("bogTuft", kt::BogTuft, -1, 0, 0);
const Feature bloomTuft = kit("bloomTuft", kt::BloomTuft, -1, 0, 0);
const Feature appleTree = kit("appleTree", kt::AppleTreeLow, kt::AppleTreeHigh, 1.3f, 3);
const Feature autumnBirch = kit("autumnBirch", kt::AutumnBirchLow, kt::AutumnBirchHigh, 1.3f, 3);
const Feature deadOak = kit("deadOak", kt::DeadOakLow, kt::DeadOakHigh, 1.1f, 3);
const Feature weepingWillow =
    kit("weepingWillow", kt::WeepingWillowLow, kt::WeepingWillowHigh, 1.4f, 3);
const Feature bamboo = kit("bamboo", kt::BambooLow, kt::BambooHigh, 1.0f, 3);
const Feature redwood = kit("redwood", kt::RedwoodLow, kt::RedwoodHigh, 1.5f, 4);
const Feature treeFern = kit("treeFern", kt::TreeFernLow, kt::TreeFernHigh, 1.2f, 3);
const Feature blueSpruce = kit("blueSpruce", kt::BlueSpruceLow, kt::BlueSpruceHigh, 1.4f, 3);
const Feature joshuaTree = kit("joshuaTree", kt::JoshuaTreeLow, kt::JoshuaTreeHigh, 1.2f, 3);
const Feature roses = kit("roses", kt::Roses, -1, 0.8f, 4);
const Feature blueberries = kit("blueberries", kt::Blueberries, -1, 0.8f, 4);
const Feature hydrangea = kit("hydrangea", kt::Hydrangea, -1, 0.8f, 4);
const Feature holly = kit("holly", kt::Holly, -1, 0.8f, 4);
const Feature pumpkins = kit("pumpkins", kt::Pumpkins, -1, 0.6f, 4);
const Feature brownMushrooms = kit("brownMushrooms", kt::BrownMushrooms, -1, 0, 0);
const Feature stump = kit("stump", kt::Stump, -1, 0.8f, 4);
const Feature mossyLog = kit("mossyLog", kt::MossyLog, -1, 0.9f, 5);
const Feature poppies = kit("poppies", kt::Poppies, -1, 0, 0);
const Feature cornflowers = kit("cornflowers", kt::Cornflowers, -1, 0, 0);
const Feature bluebells = kit("bluebells", kt::Bluebells, -1, 0, 0);
const Feature bracken = kit("bracken", kt::Bracken, -1, 0, 0);
const Feature clover = kit("clover", kt::Clover, -1, 0, 0);
const Feature menhir = kit("menhir", kt::Menhir, -1, 1.2f, 5);
const Feature rockPile = kit("rockPile", kt::RockPile, -1, 1.0f, 6);
const Feature amethyst = kit("amethyst", kt::Amethyst, -1, 0.7f, 4, {0.12f, 0.06f, 0.22f, 1});
const Feature skull = kit("skull", kt::Skull, -1, 0, 0);
const Feature driftwood = kit("driftwood", kt::Driftwood, -1, 0.6f, 0);
const Feature pricklyPear = kit("pricklyPear", kt::PricklyPear, -1, 0.7f, 4);
const Feature agave = kit("agave", kt::Agave, -1, 0.6f, 4);
const Feature tumbleweed = kit("tumbleweed", kt::Tumbleweed, -1, 0.5f, 0);
const Feature snowdrops = kit("snowdrops", kt::Snowdrops, -1, 0, 0);
const Feature frozenShrub = kit("frozenShrub", kt::FrozenShrub, -1, 0.8f, 4);
const Feature glowshrooms =
    kit("glowshrooms", kt::Glowshrooms, -1, 0, 0, {0.04f, 0.16f, 0.16f, 1});
const Feature ivy = kit("ivy", kt::Ivy, -1, 0.8f, 4);
const Feature sage = kit("sage", kt::Sage, -1, 0.8f, 4);
const Feature callaLilies = kit("callaLilies", kt::CallaLilies, -1, 0, 0);
const Feature strawberries = kit("strawberries", kt::Strawberries, -1, 0, 0);
const Feature pampas = kit("pampas", kt::Pampas, -1, 0, 0);
const Feature stoneRing = kit("stoneRing", kt::StoneRing, -1, 0, 0);

const Feature mesa = frontier("mesa", fr::MesaLow, fr::MesaHigh, 1.5f, 7);
const Feature hoodoo = frontier("hoodoo", fr::HoodooLow, fr::HoodooHigh, 1.1f, 4);
const Feature juniper = frontier("juniper", fr::JuniperLow, fr::JuniperHigh, 1.2f, 3);
const Feature taigaSpruce =
    frontier("taiga spruce", fr::TaigaSpruceLow, fr::TaigaSpruceHigh, 1.2f, 3);
const Feature larch = frontier("larch", fr::LarchLow, fr::LarchHigh, 1.2f, 3);
const Feature runeStone = frontier("rune stone", fr::RuneStoneLow, fr::RuneStoneHigh, 1.2f,
                                   5, {0.05f, 0.2f, 0.26f, 1});
const Feature ocotillo = frontier("ocotillo", fr::Ocotillo, -1, 0.6f, 3);
const Feature redRocks = frontier("red rocks", fr::RedRocks, -1, 1.0f, 6);
const Feature ribs = frontier("ribs", fr::Ribs, -1, 0, 0);
const Feature clayTuft = frontier("clay tuft", fr::ClayTuft, -1, 0, 0);
const Feature lingonberry = frontier("lingonberry", fr::Lingonberry, -1, 0.8f, 4);
const Feature lichenRock = frontier("lichen rock", fr::LichenRock, -1, 1.0f, 6);
const Feature cottonGrass = frontier("cotton grass", fr::CottonGrass, -1, 0, 0);
const Feature needleTuft = frontier("needle tuft", fr::NeedleTuft, -1, 0, 0);
const Feature shelfStump = frontier("shelf stump", fr::ShelfStump, -1, 0.8f, 4);

const Feature grave{"grave", placeGrave, t::GraveStone, -1, false, 0, 4, {0.07f, 0.15f, 0.32f, 1}};
const Feature mushrooms = town("mushrooms", tt::Mushrooms, -1, 0, 0, {0.03f, 0.12f, 0.10f, 1});
const Feature sprouts = town("sprouts", tt::Sprouts, -1, 0, 0);
const Feature log = town("log", tt::Log, -1, 0.9f, 5);
const Feature beehive = town("beehive", tt::Beehive, -1, 0.8f, 4);
const Feature sign = town("sign", tt::Sign, -1, 0.6f, 3);
const Feature sack = town("sack", tt::Sack, -1, 0.6f, 4);
const Feature crystal = dungeon("crystal", t::Crystal, -1, 0.7f, 4, {0.05f, 0.18f, 0.30f, 1});
const Feature well = town("well", tt::Well, tt::WellRoof, 1.0f, 6);
const Feature barrel = dungeon("barrel", t::Barrel, -1, 0.8f, 4);
const Feature crate = dungeon("crate", t::Crate, -1, 0.8f, 5);

} // namespace feature
