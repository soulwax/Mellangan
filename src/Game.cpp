#include "Game.hpp"

#include <spdlog/spdlog.h>

#include <cctype>
#include <cstdio>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "gfx/StatusVisual.hpp"
#include "entity/enemy/Enemy.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "level/Flavour.hpp"
#include "level/Noise.h"
#include "menu/DawnMenu.hpp"
#include "menu/DeadMenu.hpp"
#include "menu/LevelUpMenu.hpp"
#include "menu/MapMenu.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/ShrineMenu.hpp"
#include "menu/TitleMenu.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"

namespace {
std::string slurp(const std::string &path);
} // namespace

bool Game::init(SDL_Window *w, const std::string &base) {
  window = w;
  this->base = base;
  rng.state ^= SDL_GetPerformanceCounter() * 0x2545F4914F6CDD1Dull;
  if (!screen.load(base))
    return false;
  if (char *pref = SDL_GetPrefPath("soulwax", "Mellangan"))
    prefPath = pref, SDL_free(pref);
  if (!prefPath.empty() && !bench.enabled) {
    size_t size = 0;
    char *setting = static_cast<char *>(SDL_LoadFile((prefPath + "visual.txt").c_str(), &size));
    if (setting && size > 0)
      reducedParticles = setting[0] == '1';
    SDL_free(setting);
    difficulty = difficultyFrom(slurp(prefPath + "difficulty.txt"));
  }
  loadWorld();
  if (!prefPath.empty() && !bench.enabled)
    codex.load(prefPath + "codex.txt");
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
  if (!prefPath.empty() && !bench.enabled)
    codex.save(prefPath + "codex.txt");
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
  } else {
    if (input.confirm.clicked)
      level->player->cast();
    level->tick();
    if (dev && !arm.empty() && arm.front() == '=')
      level->player->pendingLevels = level->player->pendingTreasures = 0;
    if (!menuChanged && level->night.offering)
      setMenu(std::make_unique<ShrineMenu>(*this));
    else if (!menuChanged && level->player->pendingLevels > 0)
      setMenu(LevelUpMenu::open(*this));
  }
  if (menuChanged)
    menu = std::move(nextMenu), menuChanged = false;

  double k = damp(9, DT);
  xCam += (level->player->x - xCam) * k;
  yCam += (level->player->y - yCam) * k;
  hurt *= std::exp(-3.5f * DT);
  shake *= std::exp(-10 * DT);
  if (!menu)
    bannerTime = std::max(0.0f, bannerTime - DT);
  if (!menu && best > 0 && !beatBest && level->time > best)
    beatBest = true, announce("NEW BEST");
  if (bannerTime <= 0 && !banners.empty())
    banner = std::move(banners.front()), banners.pop_front(), bannerTime = 3;
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
  Structure target;
  if (input.devWarp.clicked &&
      world::nearestStructure(p.x, p.y, 8, target, Level::unvisited,
                              level.get())) {
    p.x = target.centerX();
    p.y = target.centerY();
    xCam = p.x, yCam = p.y;
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
      prefPath.empty()
          ? ""
          : prefPath + (code.empty() ? "chart.bin" : "chart-" + code + ".bin");
  level = std::make_unique<Level>(*this);
  if (!bench.enabled)
    level->chart.load(chartPath);
  xCam = level->player->x, yCam = level->player->y;
  hurt = shake = bannerTime = 0;
  banners.clear();
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
  if (dev && !arm.empty())
    armPlayer();
  setMenu(nullptr);
  best = 0, beatBest = false;
  if (level->omen >= 0)
    announce(fmt::format("{}  {}", OMENS[level->omen].name,
                         OMENS[level->omen].note));
  for (const PastWorld &w : history)
    if (w.code == worldName())
      best = w.best;
  spdlog::info("New run in world {}", worldName());
}

void Game::loadBalance() {
  balance = Balance{};
  if (bench.enabled)
    return;
  balance.load(slurp(base + "res/balance.ini"), difficulty);
  if (!prefPath.empty())
    balance.load(slurp(prefPath + "balance.ini"), difficulty);
}

void Game::setDifficulty(Difficulty d) {
  difficulty = d;
  if (!prefPath.empty() && !bench.enabled) {
    std::string name = difficultyName(d);
    SDL_SaveFile((prefPath + "difficulty.txt").c_str(), name.data(), name.size());
  }
  reseed();
}

void Game::armPlayer() {
  Player &p = *level->player;
  std::string_view list = arm;
  if (list.front() == '=') {
    list.remove_prefix(1);
    for (auto &u : p.upgrades)
      u->level = 0;
  }
  while (!list.empty()) {
    size_t comma = list.find(',');
    std::string_view name = list.substr(0, comma);
    list.remove_prefix(comma == list.npos ? list.size() : comma + 1);
    bool evolve = !name.empty() && name.back() == '!';
    if (evolve)
      name.remove_suffix(1);
    auto it = std::find_if(p.upgrades.begin(), p.upgrades.end(),
                           [&](auto &u) { return name == u->name; });
    if (it == p.upgrades.end()) {
      spdlog::warn("dev: no upgrade named {}", name);
      continue;
    }
    Upgrade &u = **it;
    while (!u.maxed())
      u.levelUp(p);
    if (evolve && u.evolvedName)
      u.evolve();
    spdlog::info("dev: armed {} at level {}", u.name, u.level);
  }
}

namespace {
std::string slurp(const std::string &path) {
  size_t size = 0;
  char *data = static_cast<char *>(SDL_LoadFile(path.c_str(), &size));
  std::string s = data ? std::string(data, size) : "";
  SDL_free(data);
  return s;
}
} // namespace

void Game::loadWorld() {
  if (prefPath.empty() || bench.enabled)
    return;
  std::string s = slurp(prefPath + "world.txt");
  if (s.rfind("random", 0) == 0)
    worldMode = WorldMode::Random;
  else if (s.rfind("daily", 0) == 0)
    worldMode = WorldMode::Daily;
  else if (s.rfind("custom ", 0) == 0)
    worldMode = WorldMode::Custom, seedCode = s.substr(7, 12);
  while (!seedCode.empty() && !std::isalnum(uint8_t(seedCode.back())))
    seedCode.pop_back();
  std::string all = slurp(prefPath + "worlds.txt");
  for (size_t at = 0; at < all.size() && int(history.size()) < HISTORY;) {
    size_t end = all.find('\n', at);
    std::string line = all.substr(at, end - at);
    at = end == std::string::npos ? all.size() : end + 1;
    char code[16] = {};
    PastWorld w{};
    int dawn = 0;
    if (std::sscanf(line.c_str(), "%15s %f %d %d", code, &w.best, &w.runs,
                    &dawn) == 4)
      w.code = code, w.dawn = dawn, history.push_back(w);
  }
}

void Game::remember(bool dawn) {
  if (bench.enabled)
    return;
  codex.runs++, codex.dawns += dawn;
  codex.bestTime = std::max(codex.bestTime, level->time);
  codex.bestStreak = std::max(codex.bestStreak, level->bestStreak);
  if (!prefPath.empty())
    codex.save(prefPath + "codex.txt");
  std::string code = worldName();
  PastWorld w{code, level->time, 1, dawn};
  for (size_t i = 0; i < history.size(); i++)
    if (history[i].code == code) {
      w.best = std::max(w.best, history[i].best);
      w.runs += history[i].runs, w.dawn |= history[i].dawn;
      history.erase(history.begin() + long(i));
      break;
    }
  history.insert(history.begin(), w);
  while (int(history.size()) > HISTORY) {
    SDL_RemovePath(
        (prefPath + "chart-" + history.back().code + ".bin").c_str());
    history.pop_back();
  }
  std::string out;
  for (const PastWorld &p : history)
    out +=
        fmt::format("{} {:.1f} {} {}\n", p.code, p.best, p.runs, int(p.dawn));
  SDL_SaveFile((prefPath + "worlds.txt").c_str(), out.data(), out.size());
}

void Game::saveWorld() const {
  if (prefPath.empty() || bench.enabled)
    return;
  std::string s = worldMode == WorldMode::Random   ? "random\n"
                  : worldMode == WorldMode::Daily  ? "daily\n"
                  : worldMode == WorldMode::Custom ? "custom " + seedCode + "\n"
                                                   : "classic\n";
  SDL_SaveFile((prefPath + "world.txt").c_str(), s.data(), s.size());
}

void Game::toTitle() {
  newGame();
  setMenu(std::make_unique<TitleMenu>(*this));
}

void Game::announce(std::string text) {
  spdlog::info("[{}] {}", level ? clockText(level->time) : "00:00", text);
  banners.push_back(std::move(text));
}

void Game::setReducedParticles(bool reduced) {
  reducedParticles = reduced;
  if (!prefPath.empty() && !bench.enabled) {
    const char *value = reduced ? "1\n" : "0\n";
    SDL_SaveFile((prefPath + "visual.txt").c_str(), value, 2);
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
  double xj = 0, yj = 0;
  if (shake > 0.05f)
    xj = rng.range(-1, 1) * shake, yj = rng.range(-1, 1) * shake;
  screen.fit(window);
  screen.xOffset = int(std::floor(xCam + xj - screen.vw / 2.0f));
  screen.yOffset = int(std::floor(yCam + yj - screen.vh / 2.0f));

  float d = level->night.daylight();
  screen.beginLight({lerpf(0.06f, 0.42f, d), lerpf(0.07f, 0.34f, d),
                     lerpf(0.15f, 0.33f, d), 1});
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
    level->renderWorldDebug(screen, xView, yView, input.mouseX, input.mouseY);
  if (!menu || menu->showHud())
    renderHud();
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

void Game::renderCompass() {
  Night &n = level->night;
  if (!n.hasBeacon || menu)
    return;
  Screen &s = screen;
  float k = s.scale, u = s.u, small = std::max(1.0f, u - 1);
  float xo = std::floor((s.w - s.vw * k) / 2),
        yo = std::floor((s.h - s.vh * k) / 2);
  float xm = xo + s.vw * k * 0.5f, ym = yo + s.vh * k * 0.5f;
  float xt = xo + float(n.beacon.centerX() - xView) * k,
        yt = yo + float(n.beacon.centerY() - 24 - yView) * k;
  float xd = xt - xm, yd = yt - ym,
        dd = std::max(1.0f, std::sqrt(xd * xd + yd * yd));
  float xh = s.vw * k * 0.5f - 18 * u, yh = s.vh * k * 0.5f - 18 * u;
  float fit = std::min({1.0f, xd != 0 ? xh / std::abs(xd) : 1e9f,
                        yd != 0 ? yh / std::abs(yd) : 1e9f});
  float xe = xm + xd * fit, ye = ym + yd * fit, xn = xd / dd, yn = yd / dd;
  if (fit >= 1)
    xn = 0, yn = 1;
  Color c = col::gold.alpha(0.75f + 0.25f * std::sin(time * 5));
  s.line(xe, ye, xe - xn * 8 * u - yn * 5 * u, ye - yn * 8 * u + xn * 5 * u,
         2 * u, c);
  s.line(xe, ye, xe - xn * 8 * u + yn * 5 * u, ye - yn * 8 * u - xn * 5 * u,
         2 * u, c);
  Player &p = *level->player;
  double xs = n.beacon.centerX() - p.x, ys = n.beacon.centerY() - p.y;
  std::string tag =
      fmt::format("SHRINE {:.0f}", std::sqrt(xs * xs + ys * ys) / 16);
  float half = Screen::textWidth(tag, small) * 0.5f + 4 * u;
  s.textCenter(tag, clampf(xe - xn * 20 * u, half, s.w - half),
               ye - yn * 20 * u - 8 * small, small, c);
}

void Game::renderMinimap() {
  Screen &s = screen;
  Player &p = *level->player;
  float u = s.u, small = std::max(1.0f, u - 1), side = 64 * u;
  float x = s.w - side - 4 * u, y = s.h - side - 6 * u;
  ChartView v{x,
              y,
              side,
              side,
              2 * u,
              p.x / (16.0 * Chart::CELL),
              p.y / (16.0 * Chart::CELL)};
  Chart::border(s, v, u);
  level->chart.draw(s, *level, v, time);
  BiomeId b =
      world::biomeAt(int(std::floor(p.x / 16)), int(std::floor(p.y / 16)));
  std::string_view name = Chart::title(b), note = FLAVOURS[b].note;
  s.text(note, s.w - Screen::textWidth(note, small) - 4 * u,
         y - 4 * u - 32 * small, small, col::gold * 0.8f);
  s.text(name, s.w - Screen::textWidth(name, small) - 4 * u,
         y - 2 * u - 16 * small, small, col::muted);
}

void Game::renderHud() {
  Screen &s = screen;
  Player &p = *level->player;
  float u = s.u, small = std::max(1.0f, u - 1), bar = 6 * u, top = bar + 5 * u;

  s.rect(0, 0, s.w, bar + 2 * u, col::panel);
  float fill = (s.w - 2 * u) * clampf(p.exp / p.expNext, 0, 1);
  s.rect(u, u, fill, bar, col::xp);
  s.rect(u, u, fill, u, {0.75f, 0.88f, 1, 0.6f});

  std::string lv = fmt::format("LV {}", p.lvl),
              kills = fmt::format("{} kills", level->kills);
  s.text(lv, s.w - Screen::textWidth(lv, u) - 4 * u, top, u, col::white);
  s.text(kills, s.w - Screen::textWidth(kills, small) - 4 * u, top + 18 * u,
         small, col::muted);
  if (level->streak >= 10) {
    std::string streak = fmt::format("x{} STREAK", level->streak);
    s.text(streak, s.w - Screen::textWidth(streak, small) - 4 * u, top + 34 * u,
           small,
           col::gold.alpha(0.5f + 0.5f * std::min(1.0f, level->streakTime)));
  }
  Night &n = level->night;
  s.textCenter(fmt::format("DAWN {}", clockText(n.left())), s.w / 2, top,
               std::round(u * 1.5f), col::white);
  if (level->omen >= 0)
    s.textCenter(OMENS[level->omen].name, s.w / 2, top + 58 * u, small,
                 OMENS[level->omen].tint);
  s.textCenter(fmt::format("{} shrine{} lit", n.lit, n.lit == 1 ? "" : "s"),
               s.w / 2, top + 26 * u, small, n.lit ? col::gold : col::muted);
  if (n.kindling > 0) {
    float kw = 120 * u, kx = (s.w - kw) / 2, ky = top + 46 * u;
    s.rect(kx, ky, kw, 4 * u, col::panel);
    s.rect(kx, ky, kw * n.kindling, 4 * u, col::gold);
  }
  renderCompass();
  renderMinimap();

  int slot[2] = {0, 0};
  for (auto &up : p.upgrades) {
    if (!up->level)
      continue;
    int row = up->weapon ? 0 : 1;
    float x = 4 * u + slot[row]++ * 20 * u, y = top + row * 24 * u;
    s.rect(x, y, 18 * u, 18 * u, {0.07f, 0.06f, 0.11f, 0.7f});
    s.sprite(up->icon, x + u, y + u, 0, 0, u, u);
    if (up->rare || up->evolved) {
      s.frame(x, y, 18 * u, 18 * u, u,
              (up->evolved ? col::xp : col::gold).alpha(0.85f));
      continue;
    }
    for (int i = 0; i < up->level; i++)
      s.rect(x + u + i * 3.4f * u, y + 19 * u, 2 * u, 2 * u, col::gold);
  }

  float sx = 4 * u, sy = top + 54 * u;
  for (int i = 0; i < int(StatusKind::Count); i++) {
    StatusKind kind = StatusKind(i);
    const StatusEffect &effect = p.status.effects[i];
    if (effect.time <= 0)
      continue;
    std::string label = fmt::format("{} {:.1f}s", statusName(kind), effect.time);
    s.text(label, sx, sy, small, statusColor(kind));
    sx += Screen::textWidth(label, small) + 8 * u;
  }

  float hw = 80 * u, hy = s.h - 12 * u;
  if (const Hero *h = p.hero) {
    float box = 22 * u, bx = 4 * u, by = hy - box - 8 * u;
    float wait = p.skillTime / std::max(0.01f, h->cooldown * p.haste);
    s.rect(bx, by, box, box, col::panel);
    s.sprite(h->sprite(), bx + 3 * u, by + 3 * u, 0, 0, u, u);
    if (wait > 0)
      s.rect(bx, by, box, box * wait, {0.02f, 0.02f, 0.05f, 0.7f});
    s.frame(bx, by, box, box, u,
            wait > 0 ? col::muted.alpha(0.4f) : h->tint.alpha(0.9f));
    s.text(h->skill, bx + box + 5 * u, by + 2 * u, small,
           wait > 0 ? col::muted : h->tint);
    s.text(wait > 0 ? fmt::format("{:.0f}s", std::ceil(p.skillTime))
                    : std::string("SPACE"),
           bx + box + 5 * u, by + 2 * u + 16 * small, small, col::muted * 0.8f);
  }
  s.rect(4 * u, hy, hw, 6 * u, col::panel);
  s.rect(5 * u, hy + u, (hw - 2 * u) * clampf(p.hp / p.maxHp, 0, 1), 4 * u,
         col::red);
  s.text(fmt::format("{}/{}", int(std::ceil(p.hp)), int(p.maxHp)), 8 * u + hw,
         hy - 5 * small, small, col::white);

  if (bannerTime > 0 && !menu) {
    float a = std::min({1.0f, bannerTime * 2, (3 - bannerTime) * 4});
    float size = 2 * u, y = s.h * 0.22f;
    float pw = Screen::textWidth(banner, size) + 24 * u, ph = 16 * size + 8 * u;
    float px = std::floor(s.w / 2 - pw / 2), py = y - 2 * u;
    s.rect(px, py, pw, ph, col::panel.alpha(0.72f * a));
    s.rect(px, py, pw, u, col::gold.alpha(0.55f * a));
    s.rect(px, py + ph - u, pw, u, col::gold.alpha(0.55f * a));
    s.textCenter(banner, s.w / 2, y, size, col::gold.alpha(a));
  }
}
