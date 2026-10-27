#include "level/Level.hpp"

#include <spdlog/spdlog.h>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"

namespace {

// Visible tiles, widened by `before` tiles up/left and `after` down/right for props that overhang their anchor.
template <class F> void eachVisibleTile(const Screen &s, F &&f, int before = 0, int after = 0) {
    int x0 = int(std::floor(s.offset.x / 16)) - before, y0 = int(std::floor(s.offset.y / 16)) - before;
    for (int y = y0; y <= y0 + before + Screen::H / 16 + 1 + after; y++)
        for (int x = x0; x <= x0 + before + Screen::W / 16 + 1 + after; x++) f(x, y);
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
        float n = world::groundShade(x, y), shade = 0.8f + 0.2f * n;
        s.color({shade, shade * 0.97f, shade * 0.93f + 0.04f * (1 - n), 1});
        s.sprite(world::groundTile(x, y), at, {0, 0});
    });
    s.color(col::white);
}

void Level::scatter(Screen &s) {
    props.clear();
    Prop p;
    eachVisibleTile(s, [&](int x, int y) {
        if (world::propAt(x, y, p)) props.push_back(p);
    }, 3, 2);
}

void Level::render(Screen &s) {
    renderFloor(s);
    sorted.clear();
    for (auto [id, o] : reg.view<Owned>().each())
        if (!o.self->removed && s.visible(o.self->pos, 48)) sorted.push_back(o.self.get());
    for (Prop &p : props) sorted.push_back(&p);
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
    scatter(s);
    player->renderLight(s);
    for (auto [id, o] : reg.view<Owned>().each())
        if (o.self.get() != player && !o.self->removed) o.self->renderLight(s);
    for (Prop &p : props) {
        if (p.light.a <= 0) continue;
        if (p.wisp) {
            float f = 0.8f + 0.2f * std::sin(time * 2 + p.pos.x * 0.37f);
            s.glow(p.pos + p.lightOffset + Vec2{0, std::sin(time * 1.7f + p.pos.y) * 2}, p.lightSize, p.light * f);
        } else {
            float f = 0.85f + 0.1f * std::sin(time * 11 + p.pos.x) + 0.05f * std::sin(time * 23 + p.pos.y);
            s.glow(p.pos + p.lightOffset, p.lightSize, p.light * f);
        }
    }
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
        Sound::horde.play(0.7f, 0);
    }
    if (t >= nextBarrel) {
        nextBarrel += 20;
        auto b = std::make_unique<SupplyBarrel>();
        b->pos = ring(rng.range(270, 300));
        level.add(std::move(b));
    }
    if (t >= nextBoss) {
        nextBoss += 150;
        bosses++;
        spawn(EnemyKind::Cyclops, ring(300));
        level.game.announce("A CYCLOPS APPROACHES");
        Sound::boss.play(0.8f, 0);
    }
}

void Spawner::spawn(EnemyKind kind, Vec2 at) {
    float hpMul = 1 + level.time / 150;
    if (kind == EnemyKind::Cyclops) hpMul *= 1 + (bosses - 1) * 0.75f;
    auto e = Enemy::create(kind, hpMul, 1 + level.time / 600);
    e->pos = at;
    if (kind != EnemyKind::Cyclops && level.time > 60 && rng.chance(0.03f + level.time / 6000)) e->makeElite();
    level.add(std::move(e));
}

Vec2 Spawner::ring(float dist) const {
    Player &p = *level.player;
    Vec2 dir = lengthSq(p.vel) > 100 && rng.chance(0.5f) ? rotate(normalized(p.vel), rng.range(-0.8f, 0.8f)) : rng.onCircle();
    return p.pos + dir * dist;
}
