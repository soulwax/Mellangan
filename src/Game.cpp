#include "Game.hpp"

#include <spdlog/spdlog.h>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"

bool Game::init(SDL_Window *w, const std::string &base) {
    window = w;
    rng.state ^= SDL_GetPerformanceCounter() * 0x2545F4914F6CDD1Dull;
    if (!screen.load(base)) return false;
    Sound::init(base);
    toTitle();
    return true;
}

void Game::shutdown() {
    vk2dRendererWait();
    menu.reset();
    nextMenu.reset();
    level.reset();
    screen.unload();
    Sound::shutdown();
}

void Game::run() {
    uint64_t last = SDL_GetPerformanceCounter();
    double acc = 0, fpsTime = 0, cpuTime = 0, perfTime = 0;
    int frames = 0, perfFrames = 0;
    while (running && !vk2dStatusFatal()) {
        uint64_t now = SDL_GetPerformanceCounter();
        double dt = double(now - last) / double(SDL_GetPerformanceFrequency());
        last = now;
        acc = std::min(acc + dt, 0.25);

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            input.handle(e, SDL_GetWindowPixelDensity(window));
        }
        uint64_t work = SDL_GetPerformanceCounter();
        for (; acc >= DT; acc -= DT) tick();
        render();
        cpuTime += double(SDL_GetPerformanceCounter() - work) / double(SDL_GetPerformanceFrequency());
        vk2dRendererPresent();

        frames++, perfFrames++;
        if ((fpsTime += dt) >= 0.5) fps = float(frames / fpsTime), frames = 0, fpsTime = 0;
        if (dev && (perfTime += dt) >= 5) {
            spdlog::info("perf {:.0f} fps, cpu {:.2f} ms/frame, {} entities, {} enemies", perfFrames / perfTime,
                         cpuTime * 1000 / perfFrames, level->reg.view<Owned>().size(), level->count<Enemy>());
            perfTime = cpuTime = 0, perfFrames = 0;
        }
    }
    if (vk2dStatusFatal()) spdlog::critical("Vulkan2D: {}", vk2dStatusMessage());
}

void Game::tick() {
    input.tick();
    time += DT;
    Sound::tick();
    if (input.fps.clicked) showFps = !showFps;
    if (input.mute.clicked) Sound::toggleMute();
    if (input.fullscreen.clicked) SDL_SetWindowFullscreen(window, !(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN));
    if (dev) devTick();

    if (menu) {
        menu->tick();
    } else if (input.escape.clicked || input.pause.clicked) {
        Sound::select.play(0.5f, 0);
        setMenu(std::make_unique<PauseMenu>(*this));
    } else {
        level->tick();
        if (!menuChanged && level->player->pendingLevels > 0) setMenu(std::make_unique<LevelUpMenu>(*this));
    }
    if (menuChanged) menu = std::move(nextMenu), menuChanged = false;

    camera = lerp(camera, level->player->pos, damp(9, DT));
    hurt *= std::exp(-3.5f * DT);
    shake *= std::exp(-10 * DT);
    bannerTime = std::max(0.0f, bannerTime - DT);
    if (bannerTime <= 0 && !banners.empty()) banner = std::move(banners.front()), banners.pop_front(), bannerTime = 3;
}

void Game::devTick() {
    Player &p = *level->player;
    if (input.devLevel.clicked) p.pendingLevels++;
    if (input.devTime.clicked) level->time += 30, spdlog::info("dev: skipped to {}", clockText(level->time));
    if (input.devGod.clicked) p.god = !p.god, spdlog::info("dev: god mode {}", p.god ? "on" : "off");
}

void Game::newGame() {
    level = std::make_unique<Level>(*this);
    camera = level->player->pos;
    hurt = shake = bannerTime = 0;
    banners.clear();
    setMenu(nullptr);
    spdlog::info("New run");
}

void Game::toTitle() {
    newGame();
    setMenu(std::make_unique<TitleMenu>(*this));
}

void Game::announce(std::string text) {
    spdlog::info("[{}] {}", clockText(level->time), text);
    banners.push_back(std::move(text));
}

void Game::onPlayerDied() {
    Player &p = *level->player;
    Sound::death.play(0.8f, 0);
    spdlog::info("Fallen after {} at level {} with {} kills", clockText(level->time), p.lvl, level->kills);
    setMenu(std::make_unique<DeadMenu>(*this));
}

void Game::render() {
    Player &p = *level->player;
    Vec2 jitter = shake > 0.05f ? Vec2{rng.range(-1, 1), rng.range(-1, 1)} * shake : Vec2{};
    screen.offset = {std::floor(camera.x + jitter.x - Screen::W / 2), std::floor(camera.y + jitter.y - Screen::H / 2)};

    screen.beginLight({0.10f, 0.12f, 0.24f, 1});
    level->renderLight(screen);
    screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level->render(screen);

    float health = p.hp / p.maxHp;
    screen.beginGui(window, {time, hurt, health < 0.3f ? 1 - health / 0.3f : 0});
    if (!menu || menu->showHud()) renderHud();
    if (menu) menu->render(screen);
    if (showFps)
        screen.text(fmt::format("{:.0f} fps  {} entities", fps, level->reg.view<Owned>().size()), 4 * screen.u,
                    screen.h - 40 * screen.u, std::max(1.0f, screen.u - 1), col::muted);
}

void Game::renderHud() {
    Screen &s = screen;
    Player &p = *level->player;
    float u = s.u, small = std::max(1.0f, u - 1), bar = 6 * u, top = bar + 5 * u;

    s.rect(0, 0, s.w, bar + 2 * u, col::panel);
    float fill = (s.w - 2 * u) * clampf(p.xp / p.xpNext, 0, 1);
    s.rect(u, u, fill, bar, col::xp);
    s.rect(u, u, fill, u, {0.75f, 0.88f, 1, 0.6f});

    std::string lv = fmt::format("LV {}", p.lvl), kills = fmt::format("{} kills", level->kills);
    s.text(lv, s.w - Screen::textWidth(lv, u) - 4 * u, top, u, col::white);
    s.text(kills, s.w - Screen::textWidth(kills, small) - 4 * u, top + 18 * u, small, col::muted);
    s.textCenter(clockText(level->time), s.w / 2, top, std::round(u * 1.5f), col::white);

    int slot[2] = {0, 0};
    for (auto &up : p.upgrades) {
        if (!up->level) continue;
        int row = up->weapon ? 0 : 1;
        float x = 4 * u + slot[row]++ * 20 * u, y = top + row * 24 * u;
        s.rect(x, y, 18 * u, 18 * u, {0.07f, 0.06f, 0.11f, 0.7f});
        s.sprite(up->icon, {x + u, y + u}, {0, 0}, {u, u});
        for (int i = 0; i < up->level; i++) s.rect(x + u + i * 3.4f * u, y + 19 * u, 2 * u, 2 * u, col::gold);
    }

    float hw = 80 * u, hy = s.h - 12 * u;
    s.rect(4 * u, hy, hw, 6 * u, col::panel);
    s.rect(5 * u, hy + u, (hw - 2 * u) * clampf(p.hp / p.maxHp, 0, 1), 4 * u, col::red);
    s.text(fmt::format("{}/{}", int(std::ceil(p.hp)), int(p.maxHp)), 8 * u + hw, hy - 5 * small, small, col::white);

    if (bannerTime > 0) {
        float a = std::min({1.0f, bannerTime * 2, (3 - bannerTime) * 4});
        s.textCenter(banner, s.w / 2, s.h * 0.22f, 2 * u, col::gold.alpha(a));
    }
}
