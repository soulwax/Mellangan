#include "entity/Entity.hpp"

void Mob::drawBody(Screen &s, int xp, int yp, int tile, bool floats, float move, Color tint, float flashAmount, Color flashColor) const {
    float sx = 1, sy = 1, rot = 0;
    if (!floats) {
        float w = std::sin(animT * 14);
        sy = 1 + (0.025f + 0.06f * move) * w;
        sx = 1 - (0.02f + 0.04f * move) * w;
        rot = std::sin(animT * 7) * 0.08f * move;
    }
    s.color(tint);
    s.flash(atlas::tile(tile), xp, yp, 8, 16, sx * scale * facing, sy * scale, rot, flashAmount, flashColor);
    s.color(col::white);
}

void Mob::drawShadow(Screen &s, int xp, int yp, bool floats) const {
    float k = scale * (floats ? 0.7f : 1.0f);
    s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, k, k);
}
