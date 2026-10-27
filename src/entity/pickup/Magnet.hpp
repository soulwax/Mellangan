#pragma once

#include "entity/pickup/Pickup.hpp"

// Pulls every gem on the map to the player.
class Magnet : public Pickup {
public:
    using Pickup::Pickup;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void take(Player &p) override;
};
