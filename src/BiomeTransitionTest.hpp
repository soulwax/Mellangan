#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

void testBiomeTransitions();
void viewDeathEffects(SDL_Window *window, const std::string &base);
void viewGui(SDL_Window *window, const std::string &base);
void viewBiomeTransition(SDL_Window *window, const std::string &base, int x,
                         int y, uint32_t seed);
