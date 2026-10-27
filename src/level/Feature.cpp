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
