#pragma once

#include "entity/Entity.hpp"

class Prop : public Entity {
public:
    struct Part {
        Sprite sprite;
        Vec2 offset;
    };
    Part parts[3]{};
    int count = 0;
    float shadow = 0, lightSize = 80;
    Color light{0, 0, 0, 0};
    Vec2 lightOffset{0, -10};
    bool wisp = true;

    void add(const Sprite &s, Vec2 offset = {}) { parts[count++] = {s, offset}; }
    void render(Screen &s) override {
        for (int i = 0; i < count; i++) s.sprite(parts[i].sprite, pos + parts[i].offset, {8, 16});
    }
    void renderShadow(Screen &s) override {
        if (shadow > 0) s.sprite(atlas::Shadow, pos + Vec2{0, -1}, {8, 4}, {shadow, 1.2f});
    }
};
