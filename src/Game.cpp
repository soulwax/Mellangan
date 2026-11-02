#include "Game.hpp"

#include <spdlog/spdlog.h>

#include <cctype>
#include <cstdio>

#include "audio/Sound.hpp"
#include "entity/Merchant.hpp"
#include "entity/Workbench.hpp"
#include "entity/Player.hpp"
#include "gfx/StatusVisual.hpp"
#include "entity/enemy/Enemy.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "level/Flavour.hpp"
#include "level/Noise.h"
#include "level/WorldDebug.hpp"
#include "menu/DawnMenu.hpp"
#include "menu/DeadMenu.hpp"
#include "menu/LevelUpMenu.hpp"
#include "menu/InventoryMenu.hpp"
#include "menu/VendorMenu.hpp"
#include "menu/CraftMenu.hpp"
#include "menu/MapMenu.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/ShrineMenu.hpp"
#include "menu/TitleMenu.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"


bool Game::init(SDL_Window *w, const std::string &base) {
  window = w;
  this->base = base;
  rng.state ^= SDL_GetPerformanceCounter() * 0x2545F4914F6CDD1Dull;
  if (!screen.load(base))
    return false;
  saves.open();
  if (!saves.dir().empty() && !bench.enabled) {
    size_t size = 0;
    char *setting = static_cast<char *>(SDL_LoadFile((saves.dir() + "visual.txt").c_str(), &size));
    if (setting && size > 0)
      reducedParticles = setting[0] == '1';
    SDL_free(setting);
    difficulty = difficultyFrom(saves.read("difficulty.txt"));
  }
  loadWorld();
  if (!saves.dir().empty() && !bench.enabled)
    codex.load(saves.dir() + "codex.txt");
  Sound::init(base);
  toTitle();
  if (bench.enabled)
    bench.start(*this);
  return true;
}

void Game::shutdown() {
  vk2dRendererWait();
  menu.reset();
  nextMenu.reset();
  if (level && !bench.enabled)
    level->chart.save(chartPath);
  if (!saves.dir().empty() && !bench.enabled)
    codex.save(saves.dir() + "codex.txt");
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
      if (e.type == SDL_EVENT_QUIT)
        running = false;
      input.handle(e, SDL_GetWindowPixelDensity(window));
    }
    uint64_t work = SDL_GetPerformanceCounter();
    int ticks = 0;
    for (; acc >= DT; acc -= DT)
      tick(), ticks++;
    uint64_t ticked = SDL_GetPerformanceCounter();
    render();
    uint64_t done = SDL_GetPerformanceCounter();
    double freq = double(SDL_GetPerformanceFrequency());
    cpuTime += double(done - work) / freq;
    bench.frame(*this, dt, ticks, double(ticked - work) / freq,
                double(done - ticked) / freq);
    vk2dRendererPresent();

    frames++, perfFrames++;
    if ((fpsTime += dt) >= 0.5)
      fps = float(frames / fpsTime), frames = 0, fpsTime = 0;
    if (dev && (perfTime += dt) >= 5) {
      spdlog::info(
          "perf {:.0f} fps, cpu {:.2f} ms/frame, {} entities, {} enemies",
          perfFrames / perfTime, cpuTime * 1000 / perfFrames,
          level->reg.view<Owned>().size(), level->count<Enemy>());
      perfTime = cpuTime = 0, perfFrames = 0;
    }
  }
  if (vk2dStatusFatal())
    spdlog::critical("Vulkan2D: {}", vk2dStatusMessage());
}

void Game::tick() {
  input.tick();
  time += DT;
  Sound::tick();
  if (level)
    Sound::ambience(level->sky.rainfall(), level->sky.gale());
  if (input.fps.clicked)
    showFps = !showFps;
  if (input.mute.clicked && !(menu && menu->typing()))
    Sound::toggleMute();
  if (input.particles.clicked && !(menu && menu->typing()))
    setReducedParticles(!reducedParticles);
  if (input.fullscreen.clicked)
    SDL_SetWindowFullscreen(
        window, !(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN));
  if (dev)
    devTick();

  if (menu) {
    menu->tick();
  } else if (input.escape.clicked || input.pause.clicked) {
    Sound::select.play(0.5f, 0);
    setMenu(std::make_unique<PauseMenu>(*this));
  } else if (input.map.clicked) {
    Sound::select.play(0.5f, 0);
    setMenu(std::make_unique<MapMenu>(*this));
  } else if (input.inventory.clicked) {
    Sound::select.play(0.5f, 0);
    if (Merchant::near(*level))
      setMenu(std::make_unique<VendorMenu>(*this));
    else if (Workbench::near(*level))
      setMenu(std::make_unique<CraftMenu>(*this));
    else
      setMenu(std::make_unique<InventoryMenu>(*this));
  } else if (freeze > 0) {
    freeze -= DT;
  } else {
    if (input.confirm.clicked)
      level->player->cast();
    level->tick();
    if (dev && start.stripsLevels())
      level->player->pendingLevels = level->player->pendingTreasures = 0;
    if (!menuChanged && level->night.offering)
      setMenu(std::make_unique<ShrineMenu>(*this));
    else if (!menuChanged && level->player->pendingLevels > 0) {
      if (levelBeat < 0)
        levelBeat = 0.3f, celebrate();
      else if ((levelBeat -= DT) <= 0)
        levelBeat = -1, setMenu(LevelUpMenu::open(*this));
    }
  }
  if (menuChanged)
    menu = std::move(nextMenu), menuChanged = false;

  camera.follow(level->player->x, level->player->y);
  hurt *= std::exp(-3.5f * DT);
  if (!menu && best > 0 && !beatBest && level->time > best)
    beatBest = true, announce("NEW BEST");
  hud.tick(!menu);
}

void Game::devTick() {
  Player &p = *level->player;
  if (input.devLevel.clicked)
    p.pendingLevels++;
  if (input.devTime.clicked)
    level->time += 30,
        spdlog::info("dev: skipped to {}", clockText(level->time));
  if (input.devGod.clicked)
    p.god = !p.god, spdlog::info("dev: god mode {}", p.god ? "on" : "off");
  if (input.devWorld.clicked)
    worldDebug = !worldDebug,
    spdlog::info("dev: world debug {}", worldDebug ? "on" : "off");
  if (input.devSky.clicked)
    level->sky.force(Front((int(level->sky.front) + 1) % int(Front::Count))),
        spdlog::info("dev: weather front {}", int(level->sky.front));
  Structure target;
  if (input.devWarp.clicked &&
      world::nearestStructure(p.x, p.y, 8, target, Level::unvisited,
                              level.get())) {
    p.x = target.centerX();
    p.y = target.centerY();
    snapCamera();
    spdlog::info("dev: warped to {} at tile ({}, {}), region ({}, {})",
                 target.name(), target.x, target.y, target.rx, target.ry);
  }
}

namespace {
const char CODE_CHARS[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

uint32_t saltOf(const std::string &code) {
  uint32_t h = 2166136261u;
  for (char c : code)
    h = (h ^ uint8_t(c)) * 16777619u;
  return code.empty() ? 0 : h | 1;
}
} // namespace

std::string Game::worldName() const {
  return worldMode == WorldMode::Classic || seedCode.empty()
             ? std::string("CLASSIC")
             : seedCode;
}

void Game::build() {
  if (level && !bench.enabled)
    level->chart.save(chartPath);
  std::string code = worldMode == WorldMode::Classic ? "" : seedCode;
  noise_reseed(saltOf(code));
  chartPath =
      saves.dir().empty()
          ? ""
          : saves.dir() + (code.empty() ? "chart.bin" : "chart-" + code + ".bin");
  level = std::make_unique<Level>(*this, rules);
  if (!bench.enabled)
    level->chart.load(chartPath);
  snapCamera();
  hurt = camera.shake = freeze = 0;
  levelBeat = -1;
  hud.clear();
}

void Game::reseed() {
  if (worldMode == WorldMode::Random && seedCode.size() != 8) {
    seedCode.clear();
    for (int i = 0; i < 8; i++)
      seedCode += CODE_CHARS[rng.next() % (sizeof CODE_CHARS - 1)];
  }
  if (worldMode == WorldMode::Classic)
    seedCode.clear();
  if (worldMode == WorldMode::Daily) {
    SDL_Time now = 0;
    SDL_DateTime d{};
    SDL_GetCurrentTime(&now);
    SDL_TimeToDateTime(now, &d, true);
    seedCode = fmt::format("D{:04}{:02}{:02}", d.year, d.month, d.day);
  }
  loadBalance();
  build();
  saveWorld();
}

void Game::newGame(bool reroll) {
  if (worldMode == WorldMode::Random && reroll)
    seedCode.clear();
  reseed();
  if (dev)
    start.apply(*level->player), snapCamera();
  setMenu(nullptr);
  best = 0, beatBest = false;
  if (level->omen >= 0)
    announce(fmt::format("{}  {}", OMENS[level->omen].name,
                         OMENS[level->omen].note));
  for (const PastWorld &w : saves.history)
    if (w.code == worldName())
      best = w.best;
  spdlog::info("New run in world {}", worldName());
}

void Game::hitStop(float seconds) {
  if (seconds < 0.1f && time - lastStop < 0.35f)
    return;
  lastStop = time;
  freeze = std::max(freeze, std::min(seconds, 0.15f));
}

void Game::celebrate() {
  Player &p = *level->player;
  p.invuln = std::max(p.invuln, 0.5f);
  level->add<Shockwave>(p.x, p.bodyY(), 4, 70, 0.4f, col::gold)->spokes = 12;
  flourish::scatter(*level, p.x, p.bodyY(), atlas::pt::HolyGlint, col::gold, 10,
                    90, -40);
  Spark::burst(*level, p.x, p.bodyY(), col::gold, 10, 120, true);
}

void Game::loadBalance() {
  balance = Balance{};
  if (bench.enabled)
    return;
  balance.load(Saves::slurp(base + "res/balance.ini"), difficulty);
  if (!saves.dir().empty())
    balance.load(saves.read("balance.ini"), difficulty);
}

void Game::setDifficulty(Difficulty d) {
  difficulty = d;
  if (!saves.dir().empty() && !bench.enabled) {
    std::string name = difficultyName(d);
    saves.write("difficulty.txt", name);
  }
  reseed();
}

void Game::snapCamera() { camera.snap(level->player->x, level->player->y); }

void Game::loadWorld() {
  if (saves.dir().empty() || bench.enabled)
    return;
  saves.loadWorld(worldMode, seedCode);
}

void Game::remember(bool dawn) {
  if (bench.enabled)
    return;
  codex.runs++, codex.dawns += dawn;
  codex.bestTime = std::max(codex.bestTime, level->time);
  codex.bestStreak = std::max(codex.bestStreak, level->bestStreak);
  if (!saves.dir().empty())
    codex.save(saves.dir() + "codex.txt");
  saves.record(worldName(), level->time, dawn);
}

void Game::saveWorld() const {
  if (saves.dir().empty() || bench.enabled)
    return;
  saves.saveWorld(worldMode, seedCode);
}

void Game::toTitle() {
  newGame();
  setMenu(std::make_unique<TitleMenu>(*this));
}

void Game::announce(std::string text) {
  spdlog::info("[{}] {}", level ? clockText(level->time) : "00:00", text);
  hud.announce(std::move(text));
}

void Game::setReducedParticles(bool reduced) {
  reducedParticles = reduced;
  if (!saves.dir().empty() && !bench.enabled) {
    const char *value = reduced ? "1\n" : "0\n";
    saves.write("visual.txt", std::string_view(value, 2));
  }
}

void Game::onPlayerDied() {
  Player &p = *level->player;
  Sound::death.play(0.8f, 0);
  remember(false);
  spdlog::info("Fallen after {} at level {} with {} kills",
               clockText(level->time), p.lvl, level->kills);
  setMenu(std::make_unique<DeadMenu>(*this));
}

void Game::onDawn() {
  Player &p = *level->player;
  p.god = true;
  remember(true);
  level->each<Enemy>([&](Enemy &e) {
    if (Debris::room())
      level->add<FlameDebris>(e.x, e.y, 6);
    e.remove();
  });
  Sound::levelUp.play(0.8f, 0);
  spdlog::info("Dawn at {} with {} shrines kindled, level {}, {} kills",
               clockText(level->time), level->night.lit, p.lvl, level->kills);
  setMenu(std::make_unique<DawnMenu>(*this));
}

void Game::render() {
  Player &p = *level->player;
  screen.fit(window);
  camera.aim(screen);

  float d = level->night.daylight();
  const Color &air = level->air;
  Color sky = level->sky.shade();
  screen.beginLight({lerpf(0.06f, 0.42f, d) * air.r * sky.r,
                     lerpf(0.07f, 0.34f, d) * air.g * sky.g,
                     lerpf(0.15f, 0.33f, d) * air.b * sky.b, 1});
  level->renderLight(screen);
  screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
  level->render(screen);
  xView = screen.xOffset, yView = screen.yOffset;

  float health = p.hp / p.maxHp;
  float pause = menu ? menu->pauseEffect() : 0.0f;
  float shrine = menu ? menu->shrineEffect() : 0.0f;
  screen.beginGui(window, {time, hurt, health < 0.3f ? 1 - health / 0.3f : 0,
                           pause, shrine});
  if (worldDebug && dev)
    WorldDebug(screen, *level, xView, yView).render(input.mouseX, input.mouseY);
  if (!menu || menu->showHud())
    hud.render(time, xView, yView);
  if (menu)
    menu->render(screen);
  if (showFps) {
    float size = std::max(1.0f, screen.u - 1);
    screen.text(fmt::format("{:.0f} fps  {} entities", fps,
                            level->reg.view<Owned>().size()),
                4 * screen.u, screen.h - 46 * screen.u - 16 * size, size,
                col::muted);
  }
}
