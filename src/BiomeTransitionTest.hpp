#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

void testBiomeTransitions();
void viewBiomeTransition(SDL_Window *window, const std::string &base, int x,
                         int y, uint32_t seed);
