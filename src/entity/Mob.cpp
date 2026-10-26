#include "entity/Entity.hpp"

void Mob::drawBody(Screen &s, int tile, bool floats, float move, Color tint, float flashAmount, Color flashColor) const {
    float bob = 0, sx = 1, sy = 1, rot = 0;
    if (floats) {
        bob = -4 + std::sin(animT * 7) * 2;
    } else {
        float w = std::sin(animT * 14);
        sy = 1 + (0.025f + 0.06f * move) * w;
        sx = 1 - (0.02f + 0.04f * move) * w;
        rot = std::sin(animT * 7) * 0.08f * move;
    }
    s.color(tint);
    s.flash(atlas::tile(tile), pos + Vec2{0, bob}, {8, 16}, {sx * scale * facing, sy * scale}, rot, flashAmount, flashColor);
    s.color(col::white);
}

void Mob::drawShadow(Screen &s, bool floats) const {
    float k = scale * (floats ? 0.7f : 1.0f);
    s.sprite(atlas::Shadow, pos + Vec2{0, 1}, {8, 4}, {k, k});
}
