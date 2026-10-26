#pragma once

#include <SDL3/SDL.h>

#include <string>

class Sound {
public:
    static Sound cast, hit, kill, slash, axe, gem, levelUp, hurt, death, chest, potion, select, confirm, boss, horde;

    static bool init(const std::string &base);
    static void shutdown();
    static void tick();
    static void toggleMute();

    void play(float volume = 1, float pitchJitter = 0.06f);

private:
    const char *file;
    float gap;
    Uint8 *data = nullptr;
    Uint32 length = 0;
    Uint64 last = 0;

    Sound(const char *file, float gap) : file(file), gap(gap) {}
    bool load(const std::string &path);
};
