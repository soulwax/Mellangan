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
  if (wantsCampaign)
    beginCampaign();
  else
    toTitle();
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
  } else if (campaign && campaign->building >= 0 &&
             (input.escape.clicked || input.build.clicked)) {
    campaign->building = -1;
    Sound::select.play(0.4f, 0);
  } else if (input.escape.clicked || input.pause.clicked) {
    Sound::select.play(0.5f, 0);
    setMenu(std::make_unique<PauseMenu>(*this));
  } else if (campaign && input.build.clicked) {
    Sound::select.play(0.5f, 0);
    setMenu(std::make_unique<BuildMenu>(*this));
  } else if (campaign && input.campaign.clicked) {
    Sound::select.play(0.5f, 0);
    setMenu(std::make_unique<TreeMenu>(*this));
  } else if (input.map.clicked) {
    Sound::select.play(0.5f, 0);
    setMenu(std::make_unique<MapMenu>(*this));
  } else if (input.inventory.clicked) {
    Sound::select.play(0.5f, 0);
    if (Elder::near(*level))
      setMenu(std::make_unique<QuestMenu>(*this));
    else if (Waypoint::near(*level) && campaign->towns().size() > 1)
      setMenu(std::make_unique<TravelMenu>(*this));
    else if (Merchant::near(*level))
      setMenu(std::make_unique<VendorMenu>(*this));
    else if (Stash::near(*level))
      setMenu(std::make_unique<StashMenu>(*this));
    else if (Workbench::near(*level) || (campaign && harvest::benchNear(*level, *campaign)))
      setMenu(std::make_unique<CraftMenu>(*this));
    else
      setMenu(std::make_unique<InventoryMenu>(*this));
  } else if (freeze > 0) {
    freeze -= DT;
  } else {
    if (campaign && campaign->building >= 0 && input.confirm.clicked) {
      int tx, ty, piece = campaign->building;
      harvest::aim(*level->player, tx, ty);
      std::string why = harvest::blocked(*level, *campaign, piece, tx, ty);
      if (why.empty() && harvest::build(*level, *campaign, piece, tx, ty)) {
        if (!harvest::affordable(*level->player, piece))
          campaign->building = -1;
        saveCampaign();
      } else {
        announce(why);
      }
    } else if (input.confirm.clicked) {
      level->player->cast();
    }
    if (Structure mine; campaign && input.gather.clicked && !campaign->delve.active &&
                        Delve::entrance(*level, *campaign, mine))
      campaign->delve.start(*level, *campaign, mine);
    else if (campaign && (input.gather.clicked ||
                          (input.gather.down && (gatherTime -= DT) <= 0))) {
      harvest::strike(*level, *campaign);
      gatherTime = GATHER_REPEAT;
    }
    level->tick();
    if (campaign)
      campaign->tick(*level);
    if (campaign && (autosave -= DT) <= 0)
      autosave = Campaign::AUTOSAVE, saveCampaign();
    if (dev && start.stripsLevels())
      level->player->pendingLevels = level->player->pendingTreasures = 0;
    if (Player &p = *level->player; campaign && p.pendingLevels > p.pendingTreasures) {
      int gained = p.pendingLevels - p.pendingTreasures;
      campaign->points += gained, p.pendingLevels = p.pendingTreasures;
      celebrate();
      announce(fmt::format("LEVEL {}  +{} SKILL POINT{}  K TO SPEND", p.lvl, gained,
                           gained == 1 ? "" : "S"));
    }
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
  endCampaign();
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
  if (campaign && campaign->hardcore) {
    spdlog::info("campaign: hardcore hero fell on day {}, the save is gone", level->night.nights);
    if (!saves.dir().empty() && !bench.enabled)
      SDL_RemovePath((saves.dir() + "campaign.txt").c_str());
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

  float d = level->night.daylight(), g = campaign ? campaign->delve.gloom : 0;
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

void Game::beginCampaign() {
  dawnRules = rules;
  campaign = std::make_unique<Campaign>();
  std::string text = saves.dir().empty() ? std::string() : saves.read("campaign.txt");
  bool fresh = text.empty() || !Campaign::load(text, *campaign);
  if (fresh) {
    *campaign = Campaign{};
    campaign->hero = start.hero >= 0 && start.hero < HERO_COUNT ? start.hero : codex.start;
    campaign->hardcore = hardcoreNext;
    for (int i = 0; i < 8; i++)
      campaign->world += CODE_CHARS[rng.next() % (sizeof CODE_CHARS - 1)];
  }
  codex.start = campaign->hero;
  worldMode = WorldMode::Custom, seedCode = campaign->world;
  rules = campaign->rules();
  loadBalance();
  build();
  campaign->enter(*level);
  if (dev) {
    for (int &m : level->player->materials)
      m += start.mats;
    level->player->gold += std::max(0, start.merchant);
  }
  for (int act = campaign->act + 1; dev && act <= std::min(start.act, ACT_COUNT); act++) {
    double rx, ry;
    if (!quests::road(act, rx, ry))
      break;
    campaign->act = act, campaign->quest = (act - 1) * QUESTS_PER_ACT;
    campaign->settle(*level, rx, ry);
    campaign->stage = QuestStage::Offered;
    level->player->x = rx, level->player->y = ry + 20;
  }
  if (dev && start.quest >= 0 && start.quest < QUEST_COUNT) {
    campaign->quest = start.quest, campaign->stage = QuestStage::Offered;
    quests::accept(*level, *campaign);
    Campaign::Town h = campaign->home();
    if (QUESTS[start.quest].goal == Objective::Escort)
      level->player->x = h.x + 20, level->player->y = std::floor(h.y / 16) * 16 + 7 * 16;
    else if (campaign->qsite)
      level->player->x = campaign->qx, level->player->y = campaign->qy + 90;
  }
  if (Structure mine; dev && start.delve &&
                      world::nearestStructure(level->player->x, level->player->y, 60, mine,
                                              [](const Structure *s, const void *) {
                                                return s->kind == StructureKind::Mine;
                                              }))
    level->player->x = mine.centerX(), level->player->y = mine.centerY() + 24;
  snapCamera();
  setMenu(nullptr);
  autosave = Campaign::AUTOSAVE;
  saveCampaign();
  spdlog::info("campaign: {} world {} as hero {}, day {}", fresh ? "new" : "loaded",
               campaign->world, campaign->hero, campaign->day);
  announce(fresh ? "ACT I  HEARTHWICK  THE BASTION HOLDS" : "HEARTHWICK STILL STANDS");
}

bool Game::campaignSaved() const {
  if (saves.dir().empty())
    return false;
  Campaign probe;
  return Campaign::load(saves.read("campaign.txt"), probe);
}

void Game::saveCampaign() {
  if (!campaign || !level || level->player->dead())
    return;
  campaign->capture(*level);
  if (!saves.dir().empty() && !bench.enabled)
    saves.write("campaign.txt", campaign->save());
}

bool Game::forget() {
  if (!campaign || !level)
    return false;
  Player &p = *level->player;
  int cost = campaign->forgetCost();
  if (p.gold < cost)
    return false;
  campaign->capture(*level);
  int before = campaign->spent(), relics = 0;
  std::vector<std::pair<std::string, int>> keep;
  for (const auto &[name, lvl] : campaign->upgrades) {
    Upgrade *u = p.upgrade(name);
    if (u && u->rare)
      keep.emplace_back(name, lvl), relics += lvl;
    else if (u && u->evolved)
      before++;
  }
  campaign->upgrades = keep, campaign->active.clear();
  campaign->gold -= cost;
  campaign->delve.abort(*level, *campaign);
  build();
  campaign->enter(*level);
  campaign->capture(*level);
  campaign->points += std::max(0, before - campaign->spent());
  snapCamera();
  setMenu(nullptr);
  saveCampaign();
  spdlog::info("campaign: forgot the path for {} gold, {} points to spend", cost,
               campaign->points);
  announce(fmt::format("YOUR PATH IS FORGOTTEN  {} SKILL POINTS", campaign->points));
  return true;
}

void Game::endCampaign() {
  if (!campaign)
    return;
  saveCampaign();
  campaign->leave();
  campaign.reset();
  rules = dawnRules;
}
