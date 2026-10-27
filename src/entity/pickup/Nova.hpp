#pragma once

#include "entity/pickup/Pickup.hpp"

// Starfire vial: blasts every enemy near the player.
class Nova : public Pickup {
public:
    using Pickup::Pickup;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void take(Player &p) override;
};
