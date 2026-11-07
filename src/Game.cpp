#include "Game.hpp"

#include <spdlog/spdlog.h>

#include <cctype>
#include <cstdio>

#include "gui/Lang.hpp"
#include "Crash.hpp"
#include "meta/Feats.hpp"
#include "meta/Heat.hpp"
#include "meta/RunLog.hpp"
#include "gfx/Palette.hpp"
#include "audio/Sound.hpp"
#include "entity/Merchant.hpp"
#include "entity/Workbench.hpp"
#include "entity/Player.hpp"
#include "level/World.hpp"
#include "gfx/StatusVisual.hpp"
#include "entity/enemy/Enemy.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "level/Flavour.hpp"
#include "level/Noise.h"
#include "level/WorldDebug.hpp"
#include "meta/Content.hpp"
#include "menu/DawnMenu.hpp"
#include "menu/DeadMenu.hpp"
#include "menu/LevelUpMenu.hpp"
#include "menu/InventoryMenu.hpp"
#include "menu/IntroMenu.hpp"
#include "menu/VendorMenu.hpp"
#include "menu/CraftMenu.hpp"
#include "menu/StashMenu.hpp"
#include "menu/TreeMenu.hpp"
#include "menu/BuildMenu.hpp"
#include "menu/QuestMenu.hpp"
#include "menu/TravelMenu.hpp"
#include "campaign/Waypoint.hpp"
#include "campaign/Elder.hpp"
#include "campaign/Harvest.hpp"
#include "campaign/Stash.hpp"
#include "menu/MapMenu.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/ShrineMenu.hpp"
#include "menu/TitleMenu.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"


bool Game::init(SDL_Window *w, const std::string &root) {
  window = w;
  base = root;
#ifndef NDEBUG
  if (auto faults = contentFaults(); !faults.empty()) {
    for (const std::string &f : faults)
      spdlog::error("content: {}", f);
    return false;
  }
#endif
  rng.state ^= SDL_GetPerformanceCounter() * 0x2545F4914F6CDD1Dull;
  if (!screen.load(base))
    return false;
  saves.open();
  if (!saves.dir().empty() && !bench.enabled) {
    std::string text = saves.read("settings.txt");
    if (!text.empty())
      settings.parse(text);
    else if (std::string visual = saves.read("visual.txt"); !visual.empty())
      settings.reduced = visual[0] == '1';
    input.parseKeys(saves.read("keys.txt"));
    difficulty = difficultyFrom(saves.read("difficulty.txt"));
  }
  loadWorld();
  if (!saves.dir().empty() && !bench.enabled)
    codex.load(saves.dir() + "codex.txt");
  Sound::init(base);
  applySettings();
  if (wantsCampaign)
    beginCampaign();
  else
    toTitle();
  if (!wantsCampaign && !dev && !bench.enabled && !readOnly && settings.intro && IntroMenu::ready(base))
    setMenu(std::make_unique<IntroMenu>(*this));
  if (menuChanged)
    menu = std::move(nextMenu), menuChanged = false;
  if (bench.enabled)
    bench.start(*this);
  return true;
}

void Game::shutdown() {
  vk2dRendererWait();
  menu.reset();
  nextMenu.reset();
  saveCampaign();
  if (campaign)
    campaign->leave();
  if (level && writes())
    level->chart.save(chartPath);
  if (writes())
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
        running = false, bench.interrupt();
      input.handle(e, SDL_GetWindowPixelDensity(window));
    }
    uint64_t work = SDL_GetPerformanceCounter();
    int ticks = 0;
    for (; acc >= DT; acc -= DT)
      tick(), ticks++;
    uint64_t ticked = SDL_GetPerformanceCounter();
    alpha = fps > 70 ? float(acc / DT) : 1;
    render();
    uint64_t done = SDL_GetPerformanceCounter();
    double freq = double(SDL_GetPerformanceFrequency());
    cpuTime += double(done - work) / freq;
    bench.frame(*this, dt, ticks, double(ticked - work) / freq,
                double(done - ticked) / freq);
    tickMs[graphAt] = float((ticked - work) * 1000 / freq);
    renderMs[graphAt] = float((done - ticked) * 1000 / freq);
    graphAt = (graphAt + 1) % GRAPH;
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
  stepped = false;
  camera.remember();
  input.tick();
  time += DT;
  Sound::tick();
  if (level)
    Sound::ambience(level->sky.rainfall(), level->sky.gale());
  hotkeys();
  if (dev)
    devTick();
  if (menu)
    menu->tick();
  else if (!route()) {
    if (freeze > 0)
      freeze -= DT;
    else
      play();
  }
  if (menuChanged)
    menu = std::move(nextMenu), menuChanged = false;

  camera.follow(level->player->x, level->player->y);
  hurt *= std::exp(-3.5f * DT);
  if (!menu && best > 0 && !beatBest && level->time > best)
    beatBest = true, announce(lang.newBest);
  hud.tick(!menu);
}

void Game::hotkeys() {
  bool typing = menu && menu->typing();
  if (input.fps.clicked)
    showFps = !showFps;
  if (input.mute.clicked && !typing)
    Sound::toggleMute();
  if (input.particles.clicked && !typing)
    settings.reduced = !settings.reduced, saveSettings();
  if (input.fullscreen.clicked)
    settings.fullscreen = !settings.fullscreen, applySettings(), saveSettings();
}

bool Game::route() {
  if (campaign && campaign->building >= 0 && (input.escape.clicked || input.build.clicked)) {
    campaign->building = -1;
    Sound::select.play(0.4f, 0);
    return true;
  }
  enum class To { Pause, Build, Tree, Map, Nearby } to;
  if (input.escape.clicked || input.pause.clicked)
    to = To::Pause;
  else if (campaign && input.build.clicked)
    to = To::Build;
  else if (campaign && input.campaign.clicked)
    to = To::Tree;
  else if (input.map.clicked)
    to = To::Map;
  else if (input.inventory.clicked)
    to = To::Nearby;
  else
    return false;
  Sound::select.play(0.5f, 0);
  switch (to) {
  case To::Pause:
    setMenu(std::make_unique<PauseMenu>(*this));
    break;
  case To::Build:
    setMenu(std::make_unique<BuildMenu>(*this));
    break;
  case To::Tree:
    setMenu(std::make_unique<TreeMenu>(*this));
    break;
  case To::Map:
    setMenu(std::make_unique<MapMenu>(*this));
    break;
  case To::Nearby:
    setMenu(nearby());
    break;
  }
  return true;
}

std::unique_ptr<Menu> Game::nearby() {
  if (Elder::near(*level))
    return std::make_unique<QuestMenu>(*this);
  if (Waypoint::near(*level) && campaign->towns().size() > 1)
    return std::make_unique<TravelMenu>(*this);
  if (Merchant::near(*level))
    return std::make_unique<VendorMenu>(*this);
  if (Stash::near(*level))
    return std::make_unique<StashMenu>(*this);
  if (Workbench::near(*level) || (campaign && harvest::benchNear(*level, *campaign)))
    return std::make_unique<CraftMenu>(*this);
  return std::make_unique<InventoryMenu>(*this);
}

void Game::play() {
  if (campaign && campaign->building >= 0 && input.confirm.clicked)
    place();
  else if (input.confirm.clicked)
    level->player->cast();
  if (campaign)
    gather();
  float was = level->time;
  level->tick(), stepped = true;
  if (int(level->time) != int(was))
    observe();
  if (campaign)
    keep();
  if (dev && start.stripsLevels())
    level->player->pendingLevels = level->player->pendingTreasures = 0;
  if (campaign)
    bankLevels();
  if (!menuChanged && level->night.offering)
    setMenu(std::make_unique<ShrineMenu>(*this));
  else if (!menuChanged && level->player->pendingLevels > 0) {
    if (levelBeat < 0)
      levelBeat = 0.3f, celebrate();
    else if ((levelBeat -= DT) <= 0)
      levelBeat = -1, setMenu(LevelUpMenu::open(*this));
  }
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
  if (level && writes())
    level->chart.save(chartPath);
  std::string code = worldMode == WorldMode::Classic ? "" : seedCode;
  noise_reseed(saltOf(code));
  static const char *const DEPTH[] = {"", "-caves", "-deep"};
  std::string depth = DEPTH[int(world::layer())];
  chartPath = saves.dir().empty()
                  ? ""
                  : saves.dir() + (code.empty() ? "chart" + depth + ".bin"
                                                : "chart-" + code + depth + ".bin");
  RunRules heated = rules;
  if (!campaign && heated.endsAtDawn) {
    Balance unused;
    applyHeat(heat, unused, heated);
  }
  level = std::make_unique<Level>(*this, heated);
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
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  build();
  saveWorld();
}

void Game::newGame(bool reroll) {
  earned.clear(), endless = false;
  if (worldMode == WorldMode::Random && reroll)
    seedCode.clear();
  reseed();
  if (dev)
    start.apply(*level->player), snapCamera();
  setMenu(nullptr);
  best = 0, beatBest = false;
  if (level->omen >= 0)
    announce(say(lang.omenBanner, tr(OMENS[level->omen].name), tr(OMENS[level->omen].note)));
  for (const PastWorld &w : saves.history)
    if (w.code == worldName())
      best = w.best;
  spdlog::info("New run in world {}", worldName());
  crashContext(fmt::format("world {}, {}", worldName(),
                           level->player->hero ? level->player->hero->name : "no hero"));
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
  if (!campaign && rules.endsAtDawn) {
    RunRules unused;
    applyHeat(heat, balance, unused);
  }
}

void Game::setDifficulty(Difficulty d) {
  difficulty = d;
  if (writes()) {
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
  if (dawn && !campaign)
    codex.heat = std::max(codex.heat, std::min(HEAT_MAX, heat + 1));
  earned = unlockFeats(*this, dawn);
  for (int f : earned)
    spdlog::info("achievement: {}", FEATS[f].title);
  if (!writes())
    return;
  codex.save(saves.dir() + "codex.txt");
  saves.record(worldName(), level->time, dawn);
  std::string log = saves.read("runs.csv");
  size_t rows = log.find('\n');
  log = std::string(RUN_HEADER) + (rows == std::string::npos ? "" : log.substr(rows + 1));
  saves.write("runs.csv", log + runRow(*this, dawn));
}

void Game::saveWorld() const {
  if (!writes())
    return;
  saves.saveWorld(worldMode, seedCode);
}

void Game::toTitle() {
  endCampaign();
  newGame();
  setMenu(std::make_unique<TitleMenu>(*this));
}

void Game::announce(std::string text) {
  spdlog::info("[{}] {}", level ? clockText(level->time) : "00:00", text);
  hud.announce(std::move(text));
}

void Game::loadLanguage() {
  static std::string loaded = "english";
  if (settings.language == loaded)
    return;
  loaded = settings.language;
  lang = Lang{};
  glossary.words.clear();
  if (loaded == "english")
    return;
  size_t size = 0;
  char *data = (char *)SDL_LoadFile((base + "res/lang/" + loaded + ".txt").c_str(), &size);
  if (!data)
    return spdlog::warn("Language {} not found, using English", loaded);
  std::string_view text(data, size);
  int strings = lang.parse(text), words = glossary.parse(text);
  SDL_free(data);
  spdlog::info("Language {}: {} of {} strings, {} content words", loaded, strings, LANG_COUNT, words);
}

void Game::applySettings() {
  loadLanguage();
  Sound::mix(settings.master / 100.0f, settings.music / 100.0f, settings.effects / 100.0f,
             settings.ambience / 100.0f);
  camera.scale = settings.shake / 100.0f;
  palette::mode = std::clamp(settings.colours, 0, palette::MODES - 1);
  screen.crisp = settings.pixel != 0;
  if (window && bool(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != bool(settings.fullscreen))
    SDL_SetWindowFullscreen(window, settings.fullscreen != 0);
  VK2DScreenMode mode = settings.vsync ? VK2D_SCREEN_MODE_VSYNC : VK2D_SCREEN_MODE_TRIPLE_BUFFER;
  if (VK2DRendererConfig config = vk2dRendererGetConfig(); window && config.screenMode != mode)
    config.screenMode = mode, vk2dRendererSetConfig(config);
}

void Game::observe() {
  Level::Trace &t = level->trace;
  Player &p = *level->player;
  t.biomes[world::biomeUnder(p.x, p.y)] += 1;
  if (int s = int(level->time - endlessFrom); endless && s > 0 && s % 60 == 0) {
    balance.enemyHp *= 1.1f, balance.enemyDamage *= 1.05f;
    announce(say(s >= 120 ? lang.darkDeepensMany : lang.darkDeepens, s / 60));
  }
  if (int(level->time) % 60 == 0)
    t.levels.push_back(p.lvl);
  if (campaign)
    checkFeats();
}

void Game::checkFeats() {
  std::vector<int> fresh = unlockFeats(*this, false, true);
  for (int f : fresh)
    announce(say(lang.achieved, tr(FEATS[f].title)));
  if (!fresh.empty() && writes())
    codex.save(saves.dir() + "codex.txt");
}

void Game::saveKeys() {
  if (writes())
    saves.write("keys.txt", input.saveKeys());
}

void Game::saveSettings() {
  if (writes())
    saves.write("settings.txt", settings.save());
}

void Game::onPlayerDied() {
  if (campaign && campaign->hardcore) {
    spdlog::info("campaign: hardcore hero fell on day {}, the save is gone", level->night.nights);
    if (writes())
      Saves::erase(saves.dir() + "campaign.txt");
    campaign->leave();
    campaign.reset();
    rules = dawnRules;
    Sound::death.play(0.8f, 0);
    return setMenu(std::make_unique<DeadMenu>(*this));
  }
  if (campaign) {
    campaign->fall(*level);
    return saveCampaign();
  }
  Player &p = *level->player;
  Sound::death.play(0.8f, 0);
  if (endless) {
    float lasted = level->time - endlessFrom;
    codex.bestEndless = std::max(codex.bestEndless, lasted);
    earned = unlockFeats(*this, false);
    if (writes())
      codex.save(saves.dir() + "codex.txt");
    spdlog::info("Fallen {} into the endless night", clockText(lasted));
    return setMenu(std::make_unique<DeadMenu>(*this));
  }
  remember(false);
  spdlog::info("Fallen after {} at level {} with {} kills",
               clockText(level->time), p.lvl, level->kills);
  setMenu(std::make_unique<DeadMenu>(*this));
}

void Game::beginEndless() {
  level->player->god = false;
  level->rules.nightLength = 1e9f;
  level->night.dawn = false;
  endless = true, endlessFrom = level->time;
  setMenu(nullptr);
  announce(lang.nightNoEnd);
  spdlog::info("Endless night from {}", clockText(level->time));
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
  camera.aim(screen, alpha);
  level->blend(stepped ? alpha : 1, screen);

  float d = level->night.daylight(), g = campaign ? campaign->delve.gloom : 0;
  if (!level->rules.sky)
    g = std::max(g, 0.45f);
  const Color &air = level->air;
  Color sky = level->sky.shade();
  screen.beginLight({lerpf(lerpf(0.06f, 0.42f, d) * air.r * sky.r, 0.035f, g),
                     lerpf(lerpf(0.07f, 0.34f, d) * air.g * sky.g, 0.028f, g),
                     lerpf(lerpf(0.15f, 0.33f, d) * air.b * sky.b, 0.03f, g), 1});
  level->renderLight(screen);
  screen.beginWorld({0.04f, 0.03f, 0.06f, 1});
  level->render(screen);
  xView = screen.xOffset, yView = screen.yOffset;

  float health = p.hp / p.maxHp;
  float pause = menu ? menu->pauseEffect() : 0.0f;
  float shrine = menu ? menu->shrineEffect() : 0.0f;
  float flash = settings.flash / 100.0f;
  screen.beginGui(window, {time, hurt * flash, (health < 0.3f ? 1 - health / 0.3f : 0) * flash,
                           pause, shrine, menu ? menu->endGlow() : Color{0, 0, 0, 0}});
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
    frameGraph(4 * screen.u, screen.h - 50 * screen.u - 16 * size);
  }
  level->unblend();
}

void Game::frameGraph(float x, float y) {
  float u = screen.u, scale = 24 * u / 16.7f, worst = 0;
  screen.rect(x, y - 36 * u, GRAPH * u, 36 * u, {0, 0, 0, 0.45f});
  for (int i = 0; i < GRAPH; i++) {
    int at = (graphAt + i) % GRAPH;
    float tk = tickMs[at], rd = renderMs[at], hi = std::min(tk + rd, 36 * u / scale);
    worst = std::max(worst, tk + rd);
    Color top = tk + rd > 16.7f ? col::red : tk + rd > 8.3f ? col::gold : col::muted;
    screen.rect(x + i * u, y - hi * scale, u, std::max(0.0f, hi - tk) * scale, top);
    screen.rect(x + i * u, y - std::min(tk, hi) * scale, u, std::min(tk, hi) * scale, col::xp);
  }
  screen.rect(x, y - 24 * u, GRAPH * u, std::max(1.0f, u * 0.5f), {1, 1, 1, 0.5f});
  float size = std::max(1.0f, u - 1);
  screen.text(fmt::format("cpu worst {:.1f} ms", worst), x + GRAPH * u + 4 * u,
              y - 8 * size, size, col::muted);
}
