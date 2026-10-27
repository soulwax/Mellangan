#include "level/LightField.hpp"

#include "level/Noise.h"

void LightField::add(double x, double y, float reach, float strength) {
    if (count < MAX) casters[count++] = {x, y, reach, strength};
}

void LightField::cast(Screen &s, double x, double y, float width, float height) const {
    for (int i = 0; i < count; i++) {
        const Caster *c = &casters[i];
        double xd = x - c->x, yd = y - c->y, d = std::sqrt(xd * xd + yd * yd);
        if (d < 6 || d > c->reach) continue;
        float fall = 1 - float(d) / c->reach, len = height * clampf(float(d) / 48, 0.35f, 2.2f);
        s.color(col::white.alpha(clampf(1.5f * c->strength * fall, 0, 1)));
        s.sprite(atlas::Shadow, x, y, 4, 4, len / 16, width, float(std::atan2(yd, xd)));
    }
    s.color(col::white);
}

float flicker(float t, float seed) {
    return 1 + 0.10f * noise_perlin(t * 3.1f, seed * 0.37f, 909) + 0.05f * noise_perlin(t * 11.7f, seed * 0.53f, 910);
}

