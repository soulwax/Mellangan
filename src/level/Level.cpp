#include "level/Level.hpp"

#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "entity/Player.hpp"

namespace {

float valueNoise(float x, float y) {
    float xf = std::floor(x), yf = std::floor(y), tx = x - xf, ty = y - yf;
    int xi = int(xf), yi = int(yf);
    auto h = [](int a, int b) { return float(hash2(a, b) & 0xFFFF) / 65535.0f; };
    tx = tx * tx * (3 - 2 * tx), ty = ty * ty * (3 - 2 * ty);
    return lerpf(lerpf(h(xi, yi), h(xi + 1, yi), tx), lerpf(h(xi, yi + 1), h(xi + 1, yi + 1), tx), ty);
}

int floorTile(uint32_t h) {
    using namespace atlas::t;
    int r = h % 100;
    return r < 12 ? SandSpeckleA : r < 22 ? SandSpeckleB : r < 25 ? SandRocks : Sand;
}

int decorTile(int x, int y) {
    using namespace atlas::t;
    if (std::abs(x) < 4 && std::abs(y) < 4) return -1;
    int r = (hash2(x * 7 + 3, y * 13 + 1) >> 4) % 1000;
    return r < 5 ? GraveCross : r < 10 ? GraveStone : r < 13 ? Barrel : r < 15 ? Crystal : -1;
}

bool hasWisp(int x, int y) {
    int d = decorTile(x, y);
    return (d == atlas::t::GraveCross || d == atlas::t::GraveStone) && (hash2(x, y) >> 16) % 3 == 0;
}

template <class F> void eachVisibleTile(const Screen &s, F &&f) {
    int x0 = int(std::floor(s.offset.x / 16)), y0 = int(std::floor(s.offset.y / 16));
    for (int y = y0; y <= y0 + Screen::H / 16 + 1; y++)
        for (int x = x0; x <= x0 + Screen::W / 16 + 1; x++) f(x, y);
}

} // namespace

Level::Level(Game &game) : game(game) {
    cellStart.assign(GRID * GRID + 1, 0);
    cellFill.assign(GRID * GRID, 0);
    player = add<Player>();
    flush();
}

Entity *Level::add(std::unique_ptr<Entity> e) {
    e->level = this;
    pending.push_back(std::move(e));
    return pending.back().get();
}

void Level::flush() {
    dead.clear();
    for (auto [id, o] : reg.view<Owned>().each())
        if (o.self->removed) dead.push_back(id);
    reg.destroy(dead.begin(), dead.end());

    for (size_t i = 0; i < pending.size(); i++) {
        std::unique_ptr<Entity> e = std::move(pending[i]);
        Entity *raw = e.get();
        raw->id = reg.create();
        reg.emplace<Owned>(raw->id, std::move(e));
        raw->onAdded();
    }
    pending.clear();
}

void Level::tick() {
    time += DT;
    if (!player->dead()) spawner.tick();
    buildGrid();
    for (auto [id, o] : reg.view<Owned>().each())
        if (!o.self->removed) o.self->tick();
    flush();
}

void Level::buildGrid() {
    gridOrigin = player->pos - Vec2{GRID * CELL * 0.5f, GRID * CELL * 0.5f};
    std::fill(cellStart.begin(), cellStart.end(), 0);
    each<Enemy>([&](Enemy &e) { cellStart[cell(e.pos.y, gridOrigin.y) * GRID + cell(e.pos.x, gridOrigin.x) + 1]++; });
    for (int i = 1; i <= GRID * GRID; i++) cellStart[i] += cellStart[i - 1];
    std::copy(cellStart.begin(), cellStart.end() - 1, cellFill.begin());
    cells.resize(cellStart.back());
    each<Enemy>([&](Enemy &e) { cells[cellFill[cell(e.pos.y, gridOrigin.y) * GRID + cell(e.pos.x, gridOrigin.x)]++] = &e; });
}

std::vector<Enemy *> Level::nearest(Vec2 p, float range, int n) {
    std::vector<std::pair<float, Enemy *>> found;
    each<Enemy>([&](Enemy &e) {
        float d = lengthSq(e.pos - p);
        if (d < range * range) found.emplace_back(d, &e);
    });
    int k = std::min(n, int(found.size()));
    std::partial_sort(found.begin(), found.begin() + k, found.end(),
                      [](auto &a, auto &b) { return a.first < b.first; });
    std::vector<Enemy *> out;
    for (int i = 0; i < k; i++) out.push_back(found[i].second);
    return out;
}

void Level::renderFloor(Screen &s) {
    eachVisibleTile(s, [&](int x, int y) {
        Vec2 at{x * 16.0f, y * 16.0f};
        float n = valueNoise(x * 0.13f, y * 0.13f), shade = 0.8f + 0.2f * n;
        s.color({shade, shade * 0.97f, shade * 0.93f + 0.04f * (1 - n), 1});
        s.sprite(atlas::tile(floorTile(hash2(x, y))), at, {0, 0});
        if (int d = decorTile(x, y); d >= 0) s.color(col::white), s.sprite(atlas::tile(d), at, {0, 0});
    });
    s.color(col::white);
}

void Level::render(Screen &s) {
    renderFloor(s);
    sorted.clear();
    for (auto [id, o] : reg.view<Owned>().each())
        if (!o.self->removed && s.visible(o.self->pos, 48)) sorted.push_back(o.self.get());
    std::sort(sorted.begin(), sorted.end(), [](Entity *a, Entity *b) {
        int la = a->layer(), lb = b->layer();
        return la != lb ? la < lb : a->pos.y < b->pos.y;
    });
    bool shadows = false;
    for (Entity *e : sorted) {
        if (!shadows && e->layer() >= 1) {
            shadows = true;
            for (Entity *m : sorted)
                if (m->layer() == 1) m->renderShadow(s);
        }
        e->render(s);
    }
}

void Level::renderLight(Screen &s) {
    player->renderLight(s);
    for (auto [id, o] : reg.view<Owned>().each())
        if (o.self.get() != player && !o.self->removed) o.self->renderLight(s);
    eachVisibleTile(s, [&](int x, int y) {
        if (!hasWisp(x, y)) return;
        float f = 0.8f + 0.2f * std::sin(time * 2 + float(hash2(x, y) % 100));
        s.glow({x * 16 + 8.0f, y * 16 + 4 + std::sin(time * 1.7f + x) * 2}, 80, Color{0.07f, 0.15f, 0.32f, 1} * f);
    });
}

void Spawner::tick() {
    float t = level.time;
    std::vector<EnemyKind> pool;
    for (int k = 0; k < int(EnemyKind::Cyclops); k++)
        if (Enemy::typeOf(EnemyKind(k)).unlockAt <= t) pool.push_back(EnemyKind(k));
    int n = int(pool.size());
    auto pick = [&] { return n > 2 && rng.chance(0.55f) ? pool[n - 1 - rng.irange(0, 1)] : pool[rng.irange(0, n - 1)]; };

    if ((timer -= DT) <= 0) {
        timer += std::max(0.06f, 0.9f - t * 0.005f);
        for (int i = 0, batch = 1 + int(t / 70); i < batch && level.count<Enemy>() < 550; i++) spawn(pick(), ring(rng.range(280, 320)));
    }
    if (t >= nextWave) {
        nextWave += 60;
        int count = std::min(64, 24 + int(t / 12));
        for (int i = 0; i < count; i++) spawn(pool.back(), level.player->pos + fromAngle(TAU * i / count) * 270);
        level.game.announce("THE HORDE CLOSES IN");
    }
    if (t >= nextBoss) {
        nextBoss += 150;
        bosses++;
        spawn(EnemyKind::Cyclops, ring(300));
        level.game.announce("A CYCLOPS APPROACHES");
    }
}

void Spawner::spawn(EnemyKind kind, Vec2 at) {
    float hpMul = 1 + level.time / 150;
    if (kind == EnemyKind::Cyclops) hpMul *= 1 + (bosses - 1) * 0.75f;
    auto e = Enemy::create(kind, hpMul, 1 + level.time / 600);
    e->pos = at;
    level.add(std::move(e));
}

Vec2 Spawner::ring(float dist) const {
    Player &p = *level.player;
    Vec2 dir = lengthSq(p.vel) > 100 && rng.chance(0.5f) ? rotate(normalized(p.vel), rng.range(-0.8f, 0.8f)) : rng.onCircle();
    return p.pos + dir * dist;
}
