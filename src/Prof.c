#include "Prof.h"

#include <SDL3/SDL_timer.h>

static Uint64 started[PROF_COUNT];
static double total[PROF_COUNT];
static const char *NAMES[PROF_COUNT] = {"think", "entities", "particles", "housekeep", "light", "floor", "sort", "draw"};

void prof_begin(int slot) { started[slot] = SDL_GetPerformanceCounter(); }

void prof_end(int slot) {
    total[slot] += (double)(SDL_GetPerformanceCounter() - started[slot]) / (double)SDL_GetPerformanceFrequency();
}

double prof_take(int slot) {
    double t = total[slot];
    total[slot] = 0;
    return t;
}

const char *prof_name(int slot) { return NAMES[slot]; }
