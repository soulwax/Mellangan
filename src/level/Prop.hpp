#pragma once

#include "SpriteObject.hpp"

class Prop : public SpriteObject {
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
    void render(Screen &s) override {
        for (int i = 0; i < count; i++) s.sprite(parts[i].sprite, xp, yp + parts[i].yo, 8, 16);
    }
    float occlusion(float &width) const override { return shadow > 0 ? (width = shadow, 4 + 12.0f * count) : (width = 0, 0.0f); }
    void renderShadow(Screen &s) override {
        if (shadow > 0) s.sprite(atlas::Shadow, xp, yp - 1, 8, 4, shadow, 1.2f);
    }
};
