#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <vector>

#include "Math.hpp"

class Game;
class Screen;
class Upgrade;

class Menu {
public:
    Game &game;
    float t = 0;

    explicit Menu(Game &game) : game(game) {}
    virtual ~Menu() = default;
    virtual void tick() { t += DT; }
    virtual void render(Screen &s) = 0;
    virtual bool showHud() const { return true; }
};

class TitleMenu : public Menu {
public:
    using Menu::Menu;
    void tick() override;
    void render(Screen &s) override;
    bool showHud() const override { return false; }
};

class PauseMenu : public Menu {
public:
    using Menu::Menu;
    void tick() override;
    void render(Screen &s) override;
};

class DeadMenu : public Menu {
public:
    using Menu::Menu;
    void tick() override;
    void render(Screen &s) override;
};

class LevelUpMenu : public Menu {
public:
    explicit LevelUpMenu(Game &game);
    void tick() override;
    void render(Screen &s) override;

private:
    std::vector<Upgrade *> choices; // nullptr: every upgrade is maxed, offer a feast
    int selected = 0;

    std::array<SDL_FRect, 3> cards() const;
    void choose(int i);
};
