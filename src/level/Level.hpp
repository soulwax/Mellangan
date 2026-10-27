#pragma once

#include <entt/entt.hpp>

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "entity/Enemy.hpp"
#include "entity/Entity.hpp"
#include "level/World.hpp"

class Game;
class Player;
class Level;

struct Owned {
    std::unique_ptr<Entity> self;
};

// Typed view onto an entity, emplaced by Entity::onAdded so systems can query by kind.
template <class T> struct Is {
    T *self;
};

class Spawner {
public:
    explicit Spawner(Level &level) : level(level) {}
    void tick();

private:
    Level &level;
    float timer = 1, nextWave = 60, nextBoss = 150, nextBarrel = 15;
    int bosses = 0;

    void spawn(EnemyKind kind, Vec2 at);
    Vec2 ring(float dist) const;
};

class Level {
public:
    Game &game;
    entt::registry reg;
    Player *player = nullptr;
    float time = 0;
    int kills = 0;

    explicit Level(Game &game);

    Entity *add(std::unique_ptr<Entity> e);
    template <class T, class... A> T *add(A &&...args) {
        return static_cast<T *>(add(std::make_unique<T>(std::forward<A>(args)...)));
    }
    template <class T> void tag(T *e) { reg.emplace_or_replace<Is<T>>(e->id, Is<T>{e}); }
    template <class T> T *get(entt::entity id) {
        if (!reg.valid(id)) return nullptr;
        auto *is = reg.try_get<Is<T>>(id);
        return is && !is->self->removed ? is->self : nullptr;
    }
    template <class T> size_t count() { return reg.view<Is<T>>().size(); }
    template <class T, class F> void each(F &&f) {
        for (auto [id, is] : reg.view<Is<T>>().each())
            if (!is.self->removed) f(*is.self);
    }
    template <class T> T *any() {
        auto v = reg.view<Is<T>>();
        return v.empty() ? nullptr : v.template get<Is<T>>(*v.begin()).self;
    }
    template <class F> void enemiesNear(Vec2 p, float r, F &&f);
    std::vector<Enemy *> nearest(Vec2 p, float range, int n);

    void tick();
    void render(Screen &s);
    // Light pass runs first each frame and also lays out the visible scenery the world pass reuses.
    void renderLight(Screen &s);
    // Dev overlay drawn in the GUI pass: regions, structure footprints, the nearest structure and the hovered tile.
    void renderWorldDebug(Screen &s, Vec2 view, Vec2 mouse);

private:
    static constexpr int GRID = 48;
    static constexpr float CELL = 28;

    Spawner spawner{*this};
    std::vector<std::unique_ptr<Entity>> pending;
    std::vector<entt::entity> dead;
    std::vector<Entity *> sorted;
    std::vector<Prop> props;
    std::vector<int> cellStart, cellFill;
    std::vector<Enemy *> cells;
    Vec2 gridOrigin;

    void flush();
    void buildGrid();
    void renderFloor(Screen &s);
    void scatter(Screen &s);
    int cell(float v, float origin) const { return std::clamp(int(std::floor((v - origin) / CELL)), 0, GRID - 1); }
};

template <class F> void Level::enemiesNear(Vec2 p, float r, F &&f) {
    int x0 = cell(p.x - r, gridOrigin.x), x1 = cell(p.x + r, gridOrigin.x);
    int y0 = cell(p.y - r, gridOrigin.y), y1 = cell(p.y + r, gridOrigin.y);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            for (int c = y * GRID + x, i = cellStart[c]; i < cellStart[c + 1]; i++)
                if (!cells[i]->removed) f(*cells[i]);
}
