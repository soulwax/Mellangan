#include "level/Feature.hpp"

namespace {

namespace tt = atlas::tt;
namespace t = atlas::t;

constexpr Color NONE{0, 0, 0, 0};

Sprite sheet(const Feature *f, int index) { return f->town ? atlas::town(index) : atlas::tile(index); }

} // namespace

bool placeStack(const Feature *f, uint32_t, Prop *p) {
    p->add(sheet(f, f->tile));
    if (f->top >= 0) p->add(sheet(f, f->top), {0, -16});
    p->shadow = f->shadow, p->light = f->light;
    return true;
}

bool placeGrave(const Feature *f, uint32_t roll, Prop *p) {
    p->add(sheet(f, roll % 10 < 3 ? t::GraveCross : f->tile));
    p->shadow = f->shadow;
    if (roll % 3 == 0) p->light = f->light;
    return true;
}

namespace feature {

const Feature oak{"oak", placeStack, tt::Trunk, tt::Crown, true, 1.3f, NONE};
const Feature autumnOak{"autumn oak", placeStack, tt::AutumnTrunk, tt::AutumnCrown, true, 1.3f, NONE};
const Feature pine{"pine", placeStack, tt::Pine, -1, true, 1.0f, NONE};
const Feature autumnPine{"autumn pine", placeStack, tt::AutumnPine, -1, true, 1.0f, NONE};
const Feature bush{"bush", placeStack, tt::Bush, -1, true, 0.9f, NONE};

const Feature grave{"grave", placeGrave, t::GraveStone, -1, false, 0, {0.07f, 0.15f, 0.32f, 1}};
const Feature mushrooms{"mushrooms", placeStack, tt::Mushrooms, -1, true, 0, {0.03f, 0.12f, 0.10f, 1}};
const Feature sprouts{"sprouts", placeStack, tt::Sprouts, -1, true, 0, NONE};
const Feature log{"log", placeStack, tt::Log, -1, true, 0.9f, NONE};
const Feature beehive{"beehive", placeStack, tt::Beehive, -1, true, 0.8f, NONE};
const Feature sign{"sign", placeStack, tt::Sign, -1, true, 0.6f, NONE};
const Feature sack{"sack", placeStack, tt::Sack, -1, true, 0.6f, NONE};
const Feature crystal{"crystal", placeStack, t::Crystal, -1, false, 0.7f, {0.05f, 0.18f, 0.30f, 1}};
const Feature well{"well", placeStack, tt::Well, tt::WellRoof, true, 1.0f, NONE};
const Feature barrel{"barrel", placeStack, t::Barrel, -1, false, 0.8f, NONE};
const Feature crate{"crate", placeStack, t::Crate, -1, false, 0.8f, NONE};

} // namespace feature
