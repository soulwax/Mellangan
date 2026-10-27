#pragma once

#include <entt/entt.hpp>

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "entity/Entity.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/LightField.hpp"
#include "level/Night.hpp"
#include "level/Spawner.hpp"
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

class Level {
public:
    Game &game;
    entt::registry reg;
    Player *player = nullptr;
    float time = 0;
    uint32_t ticks = 0;
    int kills = 0;
    Spawner spawner{*this};
    Night night{*this};
    LightField lights;
    explicit Level(Game &game);

    std::vector<std::unique_ptr<SpriteObject>> particles;

    Entity *add(std::unique_ptr<Entity> e);
    SpriteObject *spawn(std::unique_ptr<SpriteObject> p);
    template <class T, class... A> T *add(A &&...args) {
        auto p = std::make_unique<T>(std::forward<A>(args)...);
        T *raw = p.get();
        if constexpr (std::is_base_of_v<Entity, T>) add(std::unique_ptr<Entity>(std::move(p)));
        else spawn(std::move(p));
        return raw;
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
    template <class F> void enemiesNear(double x, double y, float r, F &&f);
    int gather(double x, double y, float r, Enemy **out, int max) const;
    std::vector<Enemy *> nearest(double x, double y, float range, int n);

    void tick();
    void render(Screen &s);
    void renderLight(Screen &s);
    void renderWorldDebug(Screen &s, int xView, int yView, float xMouse, float yMouse);
    void collide(double &x, double &y, float radius) const;
    bool blocks(double x, double y) const;
    void retire(entt::entity id) { dead.push_back(id); }
    bool visited(const Structure &s) const;
    static bool unvisited(const Structure *s, const void *level);

private:
    static constexpr int GRID = 48;
    static constexpr float CELL = 28;
    static constexpr int SOLID_W = Screen::W / 16 + 8, SOLID_H = Screen::H / 16 + 8;

    std::vector<std::unique_ptr<Entity>> pending;
    std::vector<entt::entity> dead;
    std::vector<SpriteObject *> drawn;
    std::vector<Enemy *> thinkers;
    std::vector<uint64_t> keys, keysTmp;
    std::vector<Prop> props;
    std::vector<int> cellStart, cellFill, cellOf;
    std::vector<Enemy *> cells;
    double xGrid = 0, yGrid = 0;
    float solids[SOLID_H * SOLID_W]{};
    int solidX = 0, solidY = 0;
    uint64_t visits[256]{};
    int visitCount = 0;

    void flush();
    void explore();
    void weather();
    void ambience();
    void buildGrid();
    void renderFloor(Screen &s);
    void scatter(Screen &s);
    void queue(SpriteObject *e);
    int cell(double v, double origin) const { return std::clamp(int(std::floor((v - origin) / CELL)), 0, GRID - 1); }
};

template <class F> void Level::enemiesNear(double x, double y, float r, F &&f) {
    int x0 = cell(x - r, xGrid), x1 = cell(x + r, xGrid);
    int y0 = cell(y - r, yGrid), y1 = cell(y + r, yGrid);
    for (int yc = y0; yc <= y1; yc++)
        for (int xc = x0; xc <= x1; xc++)
            for (int c = yc * GRID + xc, i = cellStart[c]; i < cellStart[c + 1]; i++)
                if (!cells[i]->removed) f(*cells[i]);
}
