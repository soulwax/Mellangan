#pragma once

#include <SDL3/SDL.h>
#include <spdlog/fmt/fmt.h>

#include <deque>
#include <memory>
#include <string>

#include "Bench.hpp"
#include "Input.hpp"
#include "gfx/Screen.hpp"
#include "level/Level.hpp"
#include "menu/Menu.hpp"

inline std::string clockText(float t) { return fmt::format("{:02}:{:02}", int(t) / 60, int(t) % 60); }

class Game {
public:
    SDL_Window *window = nullptr;
    Screen screen;
    Input input;
    std::unique_ptr<Level> level;
    std::unique_ptr<Menu> menu;
    bool running = true, dev = false;
    Bench bench;

    bool init(SDL_Window *window, const std::string &base);
    void shutdown();
    void run();

    // Applied once the current tick is done, so a menu can safely replace itself.
    void setMenu(std::unique_ptr<Menu> m) { nextMenu = std::move(m), menuChanged = true; }
    void newGame();
    void toTitle();
    void announce(std::string text);
    void onPlayerHurt() { hurt = 1, shake = 4; }
    void onExplosion() { shake = std::max(shake, 2.5f); }
    void onPlayerDied();

private:
    std::unique_ptr<Menu> nextMenu;
    bool menuChanged = false, showFps = false, worldDebug = false;
    float time = 0, hurt = 0, shake = 0, fps = 0, bannerTime = 0;
    std::string banner;
    std::deque<std::string> banners;
    double xCam = 0, yCam = 0;
    int xView = 0, yView = 0;

    void tick();
    void devTick();
    void render();
    void renderHud();
};
