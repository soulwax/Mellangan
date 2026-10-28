#include "entity/enemy/ai/Plans.hpp"

#include "entity/enemy/ai/Approach.hpp"
#include "entity/enemy/ai/Burrow.hpp"
#include "entity/enemy/ai/Escape.hpp"
#include "entity/enemy/ai/Falter.hpp"
#include "entity/enemy/ai/Flank.hpp"
#include "entity/enemy/ai/Follow.hpp"
#include "entity/enemy/ai/Hide.hpp"
#include "entity/enemy/ai/Howl.hpp"
#include "entity/enemy/ai/LookOut.hpp"
#include "entity/enemy/ai/Melee.hpp"
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
};

} // namespace

const Plan *planOf(EnemyKind kind) { return &PLANS[int(kind)]; }
