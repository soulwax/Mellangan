#include "level/Encounter.hpp"

#include "audio/Sound.hpp"
#include "entity/Pickup.hpp"
#include "level/Level.hpp"

namespace {

double atX(const Structure *s, int lx) { return (s->x + lx) * 16 + 8.0; }
double atY(const Structure *s, int ly) { return (s->y + ly) * 16 + 12.0; }

void cottage(Level *level, const Structure *s) { level->add<Potion>(atX(s, 2), atY(s, 4)); }

void graveyard(Level *level, const Structure *s) {
    int n = 5 + int(level->time / 40);
    for (int i = 0; i < n; i++) {
        double dir = TAU * i / n;
        level->spawner.spawn(EnemyKind::Ghost, s->centerX() + std::cos(dir) * 110, s->centerY() + std::sin(dir) * 110);
    }
    Sound::horde.play(0.5f, 0.1f);
}

void shrine(Level *level, const Structure *s) { level->add<Nova>(atX(s, s->w / 2), atY(s, s->h / 2 + 1)); }

void camp(Level *level, const Structure *s) {
    auto b = std::make_unique<SupplyBarrel>();
    b->x = atX(s, s->w / 2 + 1);
    b->y = atY(s, s->h / 2 + 1);
    level->add(std::move(b));
}

void ruin(Level *level, const Structure *s) {
    double x = atX(s, s->w / 2), y = atY(s, 2);
    if (s->seed & 1) level->add<Chest>(x, y);
    else level->add<Magnet>(x, y);
}

} // namespace

const EncounterDef ENCOUNTERS[int(StructureKind::Count)] = {
    {"A WARM HEARTH", cottage},
    {"THE DEAD STIR", graveyard},
    {"THE SHRINE HUMS", shrine},
    {"AN ABANDONED CAMP", camp},
    {"SOMETHING GLINTS IN THE RUBBLE", ruin},
};
