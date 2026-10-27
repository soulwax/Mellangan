#include "level/Prop.hpp"

void Prop::render(Screen &s) {
    for (int i = 0; i < count; i++) s.sprite(parts[i].sprite, xp, yp + parts[i].yo, 8, 16);
}

float Prop::occlusion(float &width) const {
    return shadow > 0 ? (width = shadow, 4 + 12.0f * count) : (width = 0, 0.0f);
}

void Prop::renderShadow(Screen &s) {
    if (shadow > 0) s.sprite(atlas::Shadow, xp, yp - 1, 8, 4, shadow, 1.2f);
}

