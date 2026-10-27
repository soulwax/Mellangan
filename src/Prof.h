#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum ProfSlot { PROF_THINK, PROF_ENTITIES, PROF_PARTICLES, PROF_HOUSEKEEP, PROF_LIGHT, PROF_FLOOR, PROF_SORT, PROF_DRAW, PROF_COUNT };

void prof_begin(int slot);
void prof_end(int slot);
double prof_take(int slot);
const char *prof_name(int slot);

#ifdef __cplusplus
}
#endif
