#pragma once

#include <array>
#include <cmath>

#include "entity/Entity.hpp"
class Prop : public Entity {
public:
    struct Part {
        Sprite sprite;
        Vec2 offset;
    };
    std::array<Part, 3> parts{};
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

enum class StructureKind { Cottage, Graveyard, Shrine, Camp, Ruin, Count };
struct Structure {
    StructureKind kind;
    int x, y, w, h; // footprint in tiles
    int rx, ry;     // owning region
    uint32_t seed;

    bool contains(int tx, int ty, int margin = 0) const {
        return tx >= x - margin && ty >= y - margin && tx < x + w + margin && ty < y + h + margin;
    }
    Vec2 center() const { return {(x + w * 0.5f) * 16, (y + h * 0.5f) * 16}; }
    const char *name() const;
    Color debugColor() const;
};

namespace world {

constexpr int REGION = 20;

inline int regionOf(int tile) { return int(std::floor(float(tile) / REGION)); }
bool structureIn(int rx, int ry, Structure &out);
bool structureAt(int x, int y, Structure &out, int margin = 0);
bool nearestStructure(Vec2 p, int maxRegions, Structure &out);

float forestAt(int x, int y);
float yardAt(int x, int y);
bool isDirt(int x, int y);
Sprite groundTile(int x, int y);
float groundShade(int x, int y);
bool propAt(int x, int y, Prop &p);

} // namespace world
