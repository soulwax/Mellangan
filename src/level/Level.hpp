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
template <class T> struct Is {
    T *self;
};

class Spawner {
public:
    explicit Spawner(Level &level) : level(level) {}
    void tick();
    void spawn(EnemyKind kind, Vec2 at);

private:
    Level &level;
    float timer = 1, nextWave = 60, nextBoss = 150, nextBarrel = 15;
    int bosses = 0;

    Vec2 ring(float dist) const;
};

class Level {
public:
    Game &game;
    entt::registry reg;
    Player *player = nullptr;
    float time = 0;
    int kills = 0;
    Spawner spawner{*this};

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
    int gather(Vec2 p, float r, Enemy **out, int max) const;
    std::vector<Enemy *> nearest(Vec2 p, float range, int n);

    void tick();
    void render(Screen &s);
    void renderLight(Screen &s);
    void renderWorldDebug(Screen &s, Vec2 view, Vec2 mouse);
    void collide(Vec2 &foot, float radius) const;
    bool visited(const Structure &s) const;
    static bool unvisited(const Structure *s, const void *level);

private:
    static constexpr int GRID = 48;
    static constexpr float CELL = 28;
    static constexpr int SOLID_W = Screen::W / 16 + 8, SOLID_H = Screen::H / 16 + 8;

    std::vector<std::unique_ptr<Entity>> pending;
    std::vector<entt::entity> dead;
    std::vector<Entity *> drawn;
    std::vector<uint64_t> keys, keysTmp; // draw order: layer | y | index into drawn
    std::vector<Prop> props;
    std::vector<int> cellStart, cellFill;
    std::vector<Enemy *> cells;
    Vec2 gridOrigin;
    float solids[SOLID_H * SOLID_W]{}; // prop collider radius per tile over the last scatter window
    int solidX = 0, solidY = 0;
    uint64_t visits[256]{};
    int visitCount = 0;

    void flush();
    void explore();
    void buildGrid();
    void renderFloor(Screen &s);
    void scatter(Screen &s);
    void queue(Entity *e);
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
