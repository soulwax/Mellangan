
#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <charconv>
#include <cstring>

#include "Crash.hpp"
#include "Game.hpp"
#include "test/Tests.hpp"
#include "test/Views.hpp"
#include "entity/Pool.h"
#include "Version.hpp"
#include "item/loot/Loot.hpp"
#include "item/loot/Vendor.hpp"
#include "other/Ouroboros.hpp"

static std::string initLogging(const std::string &base) {
  std::vector<spdlog::sink_ptr> sinks{
      std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
  std::string dir;
  char *pref = SDL_GetPrefPath("soulwax", "Mellangan");
  for (std::string at : {base, std::string(pref ? pref : "")}) {
    if (at.empty() || !dir.empty())
      continue;
    try {
      sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
          at + "mellangan.log", true));
      dir = at;
    } catch (const spdlog::spdlog_ex &) {
    }
  }
  SDL_free(pref);
  auto log =
      std::make_shared<spdlog::logger>("mellangan", sinks.begin(), sinks.end());
  log->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
  log->flush_on(spdlog::level::info);
#ifndef NDEBUG
  log->set_level(spdlog::level::debug);
#endif
  spdlog::set_default_logger(log);
  return dir;
}

static void consultOuroboros() {
  constexpr ouroboros::Ouroboros council;
  int n = int(SDL_GetTicks() % 1000);
  ouroboros::Augury augury;
  spdlog::info("Ouroboros: {} is {}, {} of 4 serpents agree, the augury {}", n,
               (council->*n) ? "even" : "odd", council.agreement(n),
               augury.even(n) == (council->*n) ? "concurs" : "dissents");
}

static bool parseNumber(const char *s, auto &value) {
  const char *end = s + std::strlen(s);
  auto result = std::from_chars(s, end, value);
  return result.ec == std::errc{} && result.ptr == end;
}

static int launchView(SDL_Window *window, const std::string &base, int argc,
                      char **argv) {
  using View = void (*)(SDL_Window *, const std::string &);
  const std::pair<std::string_view, View> views[] = {
      {"--capture-screenshots", captureScreenshotSuite},
      {"--test-combat-view", viewCombatReadability},
      {"--test-encounter-view", viewBiomeEncounter},
      {"--test-gui-view", viewGui},
      {"--test-effects-view", viewDeathEffects}};
  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--test-arsenal-view") {
      viewArsenal(window, base,
                  i + 1 < argc && argv[i + 1][0] != '-' ? argv[i + 1] : nullptr);
      return 0;
    }
  for (auto &[flag, run] : views)
    for (int i = 1; i < argc; i++)
      if (std::string_view(argv[i]) == flag) {
        run(window, base);
        return 0;
      }
  for (int i = 1; i + 3 < argc; i++)
    if (std::string_view(argv[i]) == "--test-biome-view") {
      int x = 0, y = 0;
      unsigned seed = 0;
      if (!parseNumber(argv[i + 1], x) || !parseNumber(argv[i + 2], y) ||
          !parseNumber(argv[i + 3], seed) || x < -1000000 || x > 1000000 ||
          y < -1000000 || y > 1000000)
        return 1;
      viewBiomeTransition(window, base, x, y, seed);
      return 0;
    }
  return -1;
}

static int runHeadless(int argc, char **argv) {
  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--test")
      return runTests();
    else if (std::string_view(argv[i]) == "--biome-map" && i + 1 < argc)
      return mapBiomes(argv[i + 1]);
    else if (std::string_view(argv[i]) == "--loot-sim" && i + 1 < argc)
      return lootSim(std::atoi(argv[i + 1]), i + 2 < argc ? std::atoi(argv[i + 2]) : 20);
    else if (std::string_view(argv[i]) == "--vendor-sim")
      return vendorSim(i + 1 < argc ? std::atoi(argv[i + 1]) : 30,
                       i + 2 < argc ? std::atoi(argv[i + 2]) : 0, true),
             0;
  return -1;
}

static SDL_Window *openWindow(const std::string &logDir) {
  SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
#ifdef __APPLE__
  flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
#endif
  SDL_Window *window = SDL_CreateWindow("Mellangan", Screen::W * 3, Screen::H * 3, flags);
  if (!window) {
    spdlog::critical("SDL_CreateWindow: {}", SDL_GetError());
    return nullptr;
  }
  VK2DRendererConfig config{VK2D_MSAA_1X, VK2D_SCREEN_MODE_VSYNC, VK2D_FILTER_TYPE_NEAREST};
  VK2DStartupOptions options{};
#ifdef __APPLE__
  std::string errorFile = logDir + "vk2derror.txt";
  options.errorFile = errorFile.c_str();
#else
  (void)logDir;
#endif
#ifndef NDEBUG
  options.enableDebug = true;
#endif
  if (vk2dRendererInit(window, config, &options) != VK2D_SUCCESS) {
    spdlog::critical("Vulkan2D init: {}", vk2dStatusMessage());
    SDL_DestroyWindow(window);
    return nullptr;
  }
  int ww, wh, pw, ph;
  SDL_GetWindowSize(window, &ww, &wh);
  SDL_GetWindowSizeInPixels(window, &pw, &ph);
  spdlog::info("{}", vk2dHostInformation());
  spdlog::info("Window {}x{}, {}x{} pixels", ww, wh, pw, ph);
  return window;
}

static int play(SDL_Window *window, const std::string &base, int argc, char **argv) {
  Game game;
  for (int i = 1; i < argc; i++) {
    game.dev |= std::string_view(argv[i]) == "--dev";
    game.bench.gate |= std::string_view(argv[i]) == "--bench-gate";
    game.bench.enabled |= std::string_view(argv[i]) == "--bench" || game.bench.gate;
    game.rules.loot |= std::string_view(argv[i]) == "--loot";
    game.wantsCampaign |= std::string_view(argv[i]) == "--campaign";
  }
  game.start.parse(argc, argv);
  if (game.dev)
    spdlog::info("Dev mode: {}", DevStart::HELP), consultOuroboros();
  int code = 1;
  if (game.init(window, base)) {
    spdlog::info("Mellangan ready");
    game.run();
    code = game.bench.failed ? 2 : 0;
  }
  game.shutdown();
  return code;
}

int main(int argc, char **argv) {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
    return 1;
  const char *basePath = SDL_GetBasePath();
  std::string base = basePath ? basePath : "";
  std::string logDir = base;
#ifdef __APPLE__
  if (char *pref = SDL_GetPrefPath("soulwax", "Mellangan"))
    logDir = pref, SDL_free(pref);
#endif
  std::string logged = initLogging(logDir);
  installCrashHandler(logged);
  spdlog::info("Mellangan v{}{}", MELLANGAN_VERSION,
               logged.empty() ? ", no log file" : ", log in " + logged + "mellangan.log");
  if (int code = runHeadless(argc, argv); code >= 0)
    return code;

  SDL_Window *window = openWindow(logDir);
  if (!window)
    return 1;
  int view = launchView(window, base, argc, argv);
  int code = view >= 0 ? view : play(window, base, argc, argv);
  vk2dRendererQuit();
  SDL_DestroyWindow(window);
  SDL_Quit();
  pool_shutdown();
  if (view < 0)
    spdlog::info("Bye");
  return code;
}

#if defined(_WIN32)
#include <windows.h>
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
  return main(__argc, __argv);
}
#endif

