#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>

void viewDeathEffects(SDL_Window *window, const std::string &base);
void viewGui(SDL_Window *window, const std::string &base);
void viewBiomeEncounter(SDL_Window *window, const std::string &base);
void viewCombatReadability(SDL_Window *window, const std::string &base);
void viewBiomeTransition(SDL_Window *window, const std::string &base, int x,
                         int y, uint32_t seed);
void viewArsenal(SDL_Window *window, const std::string &base,
                 const char *cls = nullptr);
void captureScreenshotSuite(SDL_Window *window, const std::string &base);
int mapBiomes(const char *path);
int mapCaves(const char *path);
