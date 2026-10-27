#pragma once

#include "entity/Entity.hpp"

class Prop : public Entity {
public:
    struct Part {
        Sprite sprite;
        int yo;
    };
    Part parts[3]{};
    int count = 0;
    float shadow = 0, solid = 0, lightSize = 80;
    Color light{0, 0, 0, 0};
    int lightYo = -10;
    bool wisp = true;

    void add(const Sprite &s, int yo = 0) { parts[count++] = {s, yo}; }
    void render(Screen &s, int xp, int yp) override {
        for (int i = 0; i < count; i++) s.sprite(parts[i].sprite, xp, yp + parts[i].yo, 8, 16);
    }
    void renderShadow(Screen &s, int xp, int yp) override {
        if (shadow > 0) s.sprite(atlas::Shadow, xp, yp - 1, 8, 4, shadow, 1.2f);
    }
};
