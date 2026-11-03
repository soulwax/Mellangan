#include "entity/enemy/ai/Plans.hpp"

#include "entity/enemy/ai/Approach.hpp"
#include "entity/enemy/ai/Burrow.hpp"
#include "entity/enemy/ai/Circle.hpp"
#include "entity/enemy/ai/Escape.hpp"
#include "entity/enemy/ai/Falter.hpp"
#include "entity/enemy/ai/Flank.hpp"
#include "entity/enemy/ai/Follow.hpp"
#include "entity/enemy/ai/Guard.hpp"
#include "entity/enemy/ai/Leap.hpp"
#include "entity/enemy/ai/Hide.hpp"
#include "entity/enemy/ai/Howl.hpp"
#include "entity/enemy/ai/LookOut.hpp"
#include "entity/enemy/ai/Lurk.hpp"
#include "entity/enemy/ai/Melee.hpp"
#include "entity/enemy/ai/Mend.hpp"
#include "entity/enemy/ai/Raise.hpp"
#include "entity/enemy/ai/Ranged.hpp"
#include "entity/enemy/ai/Stalk.hpp"

namespace {

const Approach CHARGE{0, {}};
const Approach FLUTTER{0, {1, 0, 0, 34, 5, false}};
const Approach OOZE{0, {0.8, 0.4, 2, 0, 0, false}};
const Approach SCUTTLE{46, {0.5, 1.2, 4, 0, 0, true}};
const Approach CLOSE{40, {}};
const Approach MARCH{48, {}};
const Approach LUMBER{70, {}};
const Approach CASTING_RANGE{200, {1, 0, 0, 28, 3, false}};
const Approach SUMMON_RANGE{190, {0.9, 0, 0, 24, 2, false}};

const LookOut WATCH{260};
const LookOut GUARD{220};
const Stalk STALK{110};
const Hide HIDE{240};
const Follow PACK{90};
const Melee NIP{0.15f, 0.18f, 2.6, 0.9f};
const Melee BITE{0.25f, 0.22f, 3.0, 1.2f};
const Melee CLEAVE{0.4f, 0.25f, 3.4, 1.6f};
const Melee SLAM{0.6f, 0.3f, 3.2, 2.4f};
const Ranged CAST{150, 210};
const Falter FALTER{0.3f};

const Flank FLANK{68};
const Howl HOWL{0.6f, 7.5f};
const Burrow BURROW{2.8f, 0.75f, 2.4f};
const Raise RAISE{0.85f, 5.5f};
const Escape ESCAPE{10.0f};
const Lurk LURK{46};
const Approach SHOOTING_RANGE{170, {1, 0, 0, 30, 3, false}};
const Approach BOW_RANGE{130, {1, 0, 0, 26, 3, false}};
const Approach CHARGE_RANGE{120, {}};
const Approach HOVER{80, {1, 0, 0, 34, 5, false}};
const Melee GORE{0.6f, 0.7f, 4.5, 2.5f};
const Melee DIVE{0.2f, 0.35f, 3.2, 1.5f};
const Melee STAB{0.2f, 0.15f, 3.4, 1.2f};
const Circle ORBIT{60, 2.5f};
const Mend MEND{150, 4};
const Approach HOP_RANGE{110, {}};
const Leap HOP{0.45f, 0.5f, 1.4f, 18};
const Approach WADE{0, {0.6, 0.3, 1.5, 0, 0, false}};
const Lurk SNAP{80};
const Melee LUNGE{0.35f, 0.5f, 5.0, 2.2f};
const Totem ROOT;
const Guard SHELL{3.5f, 0.8, 26};
const Approach DRIFT{0, {0.7, 0.6, 2.5, 0, 0, false}};
const Approach THROW_RANGE{60, {}};
const Approach HAUNT{0, {0.8, 0.5, 2, 30, 4, false}};
const Approach DRAIN_RANGE{80, {1, 0, 0, 24, 3, false}};
const Guard WARD{4.0f, 0.7, 30};
const Lurk AMBUSH{90};
const Leap POUNCE{0.25f, 0.45f, 2.2f, 22};
const Approach SNIPE{165, {1, 0, 0, 30, 3, false}};
const Approach TEND{150, {0.9, 0, 0, 24, 2, false}};
const Lurk SLUMBER{0};
const Guard BULWARK{4.5f, 0.7, 28, 1.1};
const Melee THRUST{0.35f, 0.3f, 3.6, 1.8f};
const Lurk CLING{55};

const Plan PLANS[] = {
    {{&FLUTTER}, {}},
    {{&PACK, &CHARGE}, {&FALTER}},
    {{&WATCH, &OOZE}, {}},
    {{&SCUTTLE, &NIP}, {&FALTER}},
    {{&STALK, &CLOSE, &BITE}, {&FALTER}},
    {{&OOZE}, {&HIDE}},
    {{&CASTING_RANGE, &CAST}, {&FALTER}},
    {{&MARCH, &CLEAVE}, {&FALTER}},
    {{&GUARD, &MARCH, &CLEAVE}, {&FALTER}},
    {{&LUMBER, &SLAM}, {}},
    {{&FLANK, &BITE}, {&FALTER}},
    {{&HOWL, &FLANK, &BITE}, {&FALTER}},
    {{&BURROW, &NIP}, {}},
    {{&SUMMON_RANGE, &RAISE, &CAST}, {&FALTER}},
    {{&CHARGE, &NIP}, {&FALTER}},
    {{&ESCAPE}, {}},
    {{&CHARGE}, {}},
    {{&CASTING_RANGE}, {}},
    {{&CLOSE}, {}},
    {{&FLANK}, {}},
    {{&OOZE}, {}},
    {{&LUMBER}, {}},
    {{&CHARGE}, {}},
    {{&MARCH}, {}},
    {{&SCUTTLE}, {}},
    {{&LUMBER}, {}},
    {{&LURK, &CHARGE, &BITE}, {}},
    {{&SHOOTING_RANGE}, {&FALTER}},
    {{&MARCH, &CLEAVE}, {&FALTER}},
    {{&FLANK, &BITE}, {&FALTER}},
    {{&GUARD, &MARCH, &CLEAVE}, {&FALTER}},
    {{&CLOSE, &NIP}, {&FALTER}},
    {{&CHARGE_RANGE, &GORE}, {&FALTER}},
    {{&BOW_RANGE}, {&FALTER}},
    {{&MEND, &CASTING_RANGE}, {&FALTER}},
    {{&CLOSE, &BITE}, {}},
    {{&LUMBER, &SLAM}, {}},
    {{&HOVER, &ORBIT, &DIVE}, {}},
    {{&FLANK, &STAB}, {&FALTER}},
    {{&HOP_RANGE, &HOP, &NIP}, {&FALTER}},
    {{&WADE, &BITE}, {}},
    {{&SNAP, &CHARGE_RANGE, &LUNGE}, {}},
    {{&ROOT}, {}},
    {{&SHELL, &BITE}, {&FALTER}},
    {{&DRIFT}, {}},
    {{&CASTING_RANGE, &CAST}, {&FALTER}},
    {{&THROW_RANGE, &LUMBER, &SLAM}, {}},
    {{&STALK, &HAUNT}, {}},
    {{&DRAIN_RANGE}, {&FALTER}},
    {{&WARD, &SLAM}, {}},
    {{&CHARGE, &NIP}, {}},
    {{&FLANK, &BITE}, {&FALTER}},
    {{&AMBUSH, &HOP_RANGE, &POUNCE, &STAB}, {&FALTER}},
    {{&SNIPE}, {&FALTER}},
    {{&ROOT}, {}},
    {{&FLANK, &NIP}, {}},
    {{&LUMBER, &SLAM}, {}},
    {{&SHOOTING_RANGE}, {&FALTER}},
    {{&HOVER, &ORBIT}, {}},
    {{&CLOSE, &STAB}, {&FALTER}},
    {{&TEND}, {&FALTER}},
    {{&ROOT}, {}},
    {{&OOZE}, {}},
    {{&STALK, &CLOSE, &BITE}, {&FALTER}},
    {{&SLUMBER, &LUMBER, &SLAM}, {}},
    {{&CLOSE, &BITE}, {&FALTER}},
    {{&CHARGE, &NIP}, {}},
    {{&CASTING_RANGE}, {&FALTER}},
    {{&BULWARK, &THRUST}, {}},
    {{&HOVER, &ORBIT}, {}},
    {{&FLANK, &BITE}, {&FALTER}},
    {{&TEND}, {&FALTER}},
    {{&FLUTTER}, {}},
    {{&CLING, &HOP_RANGE, &POUNCE, &NIP}, {}},
    {{&LUMBER, &SLAM}, {}},
    {{&SCUTTLE, &NIP}, {&FALTER}},
    {{&ROOT}, {}},
    {{&ROOT}, {}},
};
static_assert(sizeof(PLANS) / sizeof(PLANS[0]) == int(EnemyKind::Count));

} // namespace

const Plan *planOf(EnemyKind kind) { return &PLANS[int(kind)]; }
