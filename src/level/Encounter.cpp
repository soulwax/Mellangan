#include "level/Encounter.hpp"

#include "audio/Sound.hpp"
#include "entity/Pickup.hpp"
#include "level/Level.hpp"

namespace {

Vec2 at(const Structure *s, int lx, int ly) { return {(s->x + lx) * 16 + 8.0f, (s->y + ly) * 16 + 12.0f}; }

void cottage(Level *level, const Structure *s) { level->add<Potion>(at(s, 2, 4)); }

void graveyard(Level *level, const Structure *s) {
    int n = 5 + int(level->time / 40);
    Vec2 c = s->center();
    for (int i = 0; i < n; i++) level->spawner.spawn(EnemyKind::Ghost, c + fromAngle(TAU * i / n) * 110);
    Sound::horde.play(0.5f, 0.1f);
}

void shrine(Level *level, const Structure *s) { level->add<Nova>(at(s, s->w / 2, s->h / 2 + 1)); }

void camp(Level *level, const Structure *s) {
    auto b = std::make_unique<SupplyBarrel>();
    b->pos = at(s, s->w / 2 + 1, s->h / 2 + 1);
    level->add(std::move(b));
}

void ruin(Level *level, const Structure *s) {
    Vec2 p = at(s, s->w / 2, 2);
    if (s->seed & 1) level->add<Chest>(p);
    else level->add<Magnet>(p);
}

} // namespace

const EncounterDef ENCOUNTERS[int(StructureKind::Count)] = {
    {"A WARM HEARTH", cottage},
    {"THE DEAD STIR", graveyard},
    {"THE SHRINE HUMS", shrine},
    {"AN ABANDONED CAMP", camp},
    {"SOMETHING GLINTS IN THE RUBBLE", ruin},
};
