#pragma once

#include "gfx/Screen.hpp"

struct Caster {
    double x, y;
    float reach, strength;
};

class LightField {
public:
    static constexpr int MAX = 8;

    void clear() { count = 0; }
    void add(double x, double y, float reach, float strength);
    void cast(Screen &s, double x, double y, float width, float height) const;

private:
    Caster casters[MAX]{};
    int count = 0;
};

float flicker(float t, float seed);
