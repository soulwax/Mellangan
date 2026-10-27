#pragma once

#include "menu/Menu.hpp"

class DawnMenu : public Menu {
public:
    using Menu::Menu;
    void tick() override;
    void render(Screen &s) override;
};
