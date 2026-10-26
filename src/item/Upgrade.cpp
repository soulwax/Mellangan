#include "item/Upgrade.hpp"

#include "audio/Sound.hpp"
#include "entity/Particle.hpp"
#include "entity/Player.hpp"
#include "entity/Projectile.hpp"
#include "level/Level.hpp"

namespace {
namespace t = atlas::t;
namespace tt = atlas::tt;

int at(int lvl, int from) { return lvl >= from ? 1 : 0; }

class Staff : public Upgrade {
public:
    Staff() : Upgrade("Arcane Staff", atlas::tile(t::Staff), {"Fires bolts at the nearest foe", "+4 damage, casts faster",
                                                        "Fires one more bolt", "Bolts pierce one more foe",
                                                        "Fires one more bolt"}, true) {}

    void tick(Player &p) override {
        if ((cd -= DT) > 0) return;
        int n = 1 + at(level, 3) + at(level, 5);
        auto targets = p.level->nearest(p.pos, 250, n);
        if (targets.empty()) return void(cd = 0);
        cd = (level >= 2 ? 0.85f : 1.05f) * p.haste;
        for (int i = 0; i < n; i++) {
            Enemy *e = targets[i % targets.size()];
            float spread = i < int(targets.size()) ? 0 : rng.range(-0.35f, 0.35f);
            Vec2 dir = rotate(normalized(e->body() - p.body()), spread);
            p.level->add<Bolt>(p.body(), dir * 230, (level >= 2 ? 14.0f : 10.0f) * p.might, 1 + at(level, 4), e->id);
        }
        Spark::burst(*p.level, p.body(), {0.45f, 0.8f, 1, 1}, 4, 40, true);
        Sound::cast.play(0.3f, 0.1f);
    }

private:
    float cd = 0.4f;
};

class Blade : public Upgrade {
public:
    Blade() : Upgrade("Moon Blade", atlas::tile(t::Sword), {"Cleaves the side you face", "Also cleaves behind you",
                                            "+6 damage, wider arc", "+6 damage, faster swings",
                                            "+6 damage, wider arc"}, true) {}

    void tick(Player &p) override {
        if ((cd -= DT) > 0) return;
        cd = (level >= 4 ? 1.05f : 1.3f) * p.haste;
        float dmg = (16.0f + 6 * (at(level, 3) + at(level, 4) + at(level, 5))) * p.might;
        float scale = 1 + 0.2f * (at(level, 3) + at(level, 5));
        p.level->add<Slash>(p, p.facing, dmg, scale);
        if (level >= 2) p.level->add<Slash>(p, -p.facing, dmg, scale);
        Sound::slash.play(0.5f);
    }

private:
    float cd = 0.6f;
};

class Halo;

class HaloField : public Entity {
public:
    HaloField(const Halo &halo, const Player &p) : halo(halo), player(p) {}
    int layer() const override { return 0; }
    void tick() override { pos = player.pos; }
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    const Halo &halo;
    const Player &player;
};

class Halo : public Upgrade {
public:
    float pulse = 0;

    Halo() : Upgrade("Warding Halo", atlas::tile(t::PotionGrey), {"Scorches foes that get close", "Wider and hotter", "Wider and hotter",
                                              "Wider, pulses faster", "Wider and hotter"}, true) {}

    float range() const { return 30.0f + 7 * (level - 1); }

    void levelUp(Player &p) override {
        Upgrade::levelUp(p);
        if (level == 1) p.level->add<HaloField>(*this, p);
    }

    void tick(Player &p) override {
        pulse *= std::exp(-6 * DT);
        if ((timer -= DT) > 0) return;
        timer = (level >= 4 ? 0.38f : 0.5f) * p.haste;
        pulse = 1;
        float r = range(), dmg = (5.0f + 2 * (at(level, 2) + at(level, 3) + at(level, 5))) * p.might;
        p.level->enemiesNear(p.pos, r + 24, [&](Enemy &e) {
            float rr = r + e.radius;
            if (lengthSq(e.body() - p.body()) < rr * rr) e.hurt(dmg, normalized(e.pos - p.pos), 25);
        });
    }

private:
    float timer = 0;
};

void HaloField::render(Screen &s) {
    s.color({0.7f, 0.55f, 1, 0.22f + 0.35f * halo.pulse});
    s.center(atlas::Ring, pos + Vec2{0, -4}, halo.range() * 2 / atlas::Ring.w * (1 + 0.05f * halo.pulse), level->time * 0.6f);
    s.color(col::white);
}

void HaloField::renderLight(Screen &s) {
    s.glow(pos, halo.range() * 2.8f, Color{0.10f, 0.05f, 0.20f, 1} * (1 + halo.pulse * 0.6f));
}

class Knives;

class Knife : public Entity {
public:
    Knife(const Knives &knives, const Player &p, int index) : knives(knives), player(p), index(index) {}
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override { s.center(atlas::tile(t::Dagger), pos, 1, angle + PI); }
    void renderLight(Screen &s) override { s.glow(pos, 30, {0.08f, 0.08f, 0.12f, 1}); }

private:
    const Knives &knives;
    const Player &player;
    int index;
    float angle = 0;
};

class Knives : public Upgrade {
public:
    float angle = 0;

    Knives() : Upgrade("Orbiting Knives", atlas::tile(t::Dagger), {"Two knives circle around you", "One more knife",
                                                   "One more knife, +4 damage", "One more knife",
                                                   "Faster spin, +4 damage"}, true) {}

    int count() const { return 2 + at(level, 2) + at(level, 3) + at(level, 4); }
    float damage() const { return 9.0f + 4 * (at(level, 3) + at(level, 5)); }

    void levelUp(Player &p) override {
        Upgrade::levelUp(p);
        while (spawned < count()) p.level->add<Knife>(*this, p, spawned++);
    }

    void tick(Player &) override { angle += (level >= 5 ? 4.6f : 3.4f) * DT; }

private:
    int spawned = 0;
};

void Knife::tick() {
    angle = knives.angle + TAU * index / knives.count();
    pos = player.body() + fromAngle(angle) * 34;
    float dmg = knives.damage() * player.might;
    level->enemiesNear(pos, 32, [&](Enemy &e) {
        float rr = 6 + e.radius;
        if (e.knifeCooldown > 0 || lengthSq(e.body() - pos) >= rr * rr) return;
        e.hurt(dmg, normalized(e.pos - player.pos), 70);
        e.knifeCooldown = 0.45f;
        Spark::burst(*level, pos, col::white, 2, 50, false);
    });
}

class AxeThrow : public Upgrade {
public:
    AxeThrow() : Upgrade("Hurled Axe", atlas::tile(t::Axe), {"Lobs a heavy spinning axe", "+10 damage", "Throws one more axe",
                                                       "+10 damage, more pierce", "Throws one more axe"}, true) {}

    void tick(Player &p) override {
        if ((cd -= DT) > 0) return;
        cd = 2.3f * p.haste;
        Sound::axe.play(0.45f);
        int n = 1 + at(level, 3) + at(level, 5);
        float dmg = (26.0f + 10 * (at(level, 2) + at(level, 4))) * p.might;
        for (int i = 0; i < n; i++) {
            Vec2 vel{p.facing * rng.range(25, 70) + (i - (n - 1) * 0.5f) * 55, -rng.range(230, 260)};
            p.level->add<::Axe>(p.body(), vel, dmg, 5 + 4 * at(level, 4), p.facing * 14);
        }
    }

private:
    float cd = 1;
};

class StormRod : public Upgrade {
public:
    StormRod() : Upgrade("Storm Rod", atlas::tile(t::StormStaff), {"Calls lightning on random foes", "One more strike",
                                                                  "+12 damage, wider blast", "One more strike, faster",
                                                                  "Two more strikes"}, true) {}

    void tick(Player &p) override {
        if ((cd -= DT) > 0) return;
        std::vector<Enemy *> pool;
        p.level->each<Enemy>([&](Enemy &e) {
            if (lengthSq(e.pos - p.pos) < 230 * 230) pool.push_back(&e);
        });
        if (pool.empty()) return void(cd = 0.2f);
        cd = (level >= 4 ? 1.6f : 2.1f) * p.haste;
        int n = 1 + at(level, 2) + at(level, 4) + 2 * at(level, 5);
        float dmg = (28.0f + 12 * at(level, 3)) * p.might, r = 14.0f + 8 * at(level, 3);
        for (int i = 0; i < n; i++) p.level->add<Strike>(pool[rng.irange(0, int(pool.size()) - 1)]->pos, dmg, r);
        Sound::thunder.play(0.35f);
    }

private:
    float cd = 1.5f;
};

class FireBomb : public Upgrade {
public:
    FireBomb() : Upgrade("Fire Bomb", atlas::town(tt::Bomb), {"Lobs a bomb that leaves flames", "Bigger, longer fire",
                                                             "Throws one more bomb", "Hotter flames, faster throws",
                                                             "Throws one more bomb"}, true) {}

    void tick(Player &p) override {
        if ((cd -= DT) > 0) return;
        cd = (level >= 4 ? 2.4f : 3.0f) * p.haste;
        int n = 1 + at(level, 3) + at(level, 5);
        auto targets = p.level->nearest(p.pos, 200, 12);
        for (int i = 0; i < n; i++) {
            Vec2 to = targets.empty() ? p.pos + rng.onCircle() * rng.range(50, 110)
                                      : targets[rng.irange(0, int(targets.size()) - 1)]->pos;
            p.level->add<Bomb>(p.body(), to, 20 * p.might, 22.0f + 6 * at(level, 2), (5.0f + 4 * at(level, 4)) * p.might,
                               2.5f + at(level, 2));
        }
        Sound::axe.play(0.4f);
    }

private:
    float cd = 1.2f;
};

class Bow : public Upgrade {
public:
    Bow() : Upgrade("Hunter's Bow", atlas::town(tt::Bow), {"Looses arrows where you walk", "+6 damage, more pierce",
                                                          "Fires a second arrow", "Faster draw, more pierce",
                                                          "Fires a third arrow"}, true) {}

    void tick(Player &p) override {
        if ((cd -= DT) > 0) return;
        cd = (level >= 4 ? 0.6f : 0.85f) * p.haste;
        int n = 1 + at(level, 3) + at(level, 5), pierce = 3 + 2 * at(level, 2) + 2 * at(level, 4);
        float dmg = (13.0f + 6 * at(level, 2)) * p.might, aim = std::atan2(p.aim.y, p.aim.x);
        for (int i = 0; i < n; i++)
            p.level->add<Arrow>(p.body(), fromAngle(aim + (i - (n - 1) * 0.5f) * 0.14f) * 320, dmg, pierce);
        Sound::bow.play(0.35f);
    }

private:
    float cd = 0.5f;
};

class Scythe : public Upgrade {
public:
    Scythe() : Upgrade("Reaper's Scythe", atlas::town(tt::Scythe), {"Sweeps a full circle around you", "+10 damage",
                                                                    "Longer reach", "+10 damage, faster sweeps",
                                                                    "Longer reach, +10 damage"}, true) {}

    void tick(Player &p) override {
        if ((cd -= DT) > 0) return;
        cd = (level >= 4 ? 2.0f : 2.8f) * p.haste;
        p.level->add<Sweep>(p, 28.0f + 8 * (at(level, 3) + at(level, 5)),
                            (24.0f + 10 * (at(level, 2) + at(level, 4) + at(level, 5))) * p.might);
        Sound::scythe.play(0.5f);
    }

private:
    float cd = 1;
};

class Passive : public Upgrade {
public:
    Passive(const char *name, Sprite icon, const char *desc, float Player::*stat, float amount)
        : Upgrade(name, icon, {desc, desc, desc, desc, desc}, false), stat(stat), amount(amount) {}

    void levelUp(Player &p) override {
        Upgrade::levelUp(p);
        p.*stat += amount;
        if (stat == &Player::maxHp) p.hp += amount;
    }

private:
    float Player::*stat;
    float amount;
};

} // namespace

std::vector<std::unique_ptr<Upgrade>> Upgrade::all() {
    std::vector<std::unique_ptr<Upgrade>> u;
    u.push_back(std::make_unique<Staff>());
    u.push_back(std::make_unique<Blade>());
    u.push_back(std::make_unique<Halo>());
    u.push_back(std::make_unique<Knives>());
    u.push_back(std::make_unique<AxeThrow>());
    u.push_back(std::make_unique<StormRod>());
    u.push_back(std::make_unique<FireBomb>());
    u.push_back(std::make_unique<Bow>());
    u.push_back(std::make_unique<Scythe>());
    auto passive = [&](const char *name, Sprite icon, const char *desc, float Player::*stat, float amount) {
        u.push_back(std::make_unique<Passive>(name, icon, desc, stat, amount));
    };
    passive("Quicksilver", atlas::tile(t::PotionBlue), "+10% movement speed", &Player::speed, 7.4f);
    passive("Heartroot", atlas::tile(t::PotionRed), "+20 max health", &Player::maxHp, 20);
    passive("Ogre Hammer", atlas::tile(t::Hammer), "+10% damage", &Player::might, 0.1f);
    passive("Hourglass Tonic", atlas::tile(t::PotionGreen), "-7% weapon cooldowns", &Player::haste, -0.07f);
    passive("Lodestone", atlas::tile(t::Coin), "+30% pickup range", &Player::magnet, 8.4f);
    passive("Iron Buckler", atlas::tile(t::Shield), "Take 1 less damage per hit", &Player::armor, 1);
    passive("Troll Blood", atlas::tile(t::VialGreen), "Regenerate 0.3 health/second", &Player::regen, 0.3f);
    passive("Lucky Key", atlas::town(tt::Key), "+4% critical hit chance", &Player::luck, 0.04f);
    return u;
}
