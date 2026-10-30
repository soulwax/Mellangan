
#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <charconv>
#include <cstring>

#include "Crash.hpp"
#include "Game.hpp"
#include "Test.hpp"
#include "BiomeTransitionTest.hpp"
#include "entity/Pool.h"
#include "Version.hpp"
#include "other/Ouroboros.hpp"

static void initLogging(const std::string &base) {
  std::vector<spdlog::sink_ptr> sinks{
      std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
  try {
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        base + "mellangan.log", true));
  } catch (const spdlog::spdlog_ex &) {
  }
  auto log =
      std::make_shared<spdlog::logger>("mellangan", sinks.begin(), sinks.end());
  log->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
  log->flush_on(spdlog::level::info);
#ifndef NDEBUG
  log->set_level(spdlog::level::debug);
#endif
  spdlog::set_default_logger(log);
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
  initLogging(logDir);
  installCrashHandler();
  spdlog::info("Mellangan v{}", MELLANGAN_VERSION);

  SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
#ifdef __APPLE__
  flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
#endif
  SDL_Window *window =
      SDL_CreateWindow("Mellangan", Screen::W * 3, Screen::H * 3, flags);
  if (!window) {
    spdlog::critical("SDL_CreateWindow: {}", SDL_GetError());
    return 1;
  }

  VK2DRendererConfig config{VK2D_MSAA_1X, VK2D_SCREEN_MODE_VSYNC,
                            VK2D_FILTER_TYPE_NEAREST};
  VK2DStartupOptions options{};
#ifdef __APPLE__
  std::string errorFile = logDir + "vk2derror.txt";
  options.errorFile = errorFile.c_str();
#endif
#ifndef NDEBUG
  options.enableDebug = true;
#endif
  if (vk2dRendererInit(window, config, &options) != VK2D_SUCCESS) {
    spdlog::critical("Vulkan2D init: {}", vk2dStatusMessage());
    return 1;
  }
  int ww, wh, pw, ph;
  SDL_GetWindowSize(window, &ww, &wh);
  SDL_GetWindowSizeInPixels(window, &pw, &ph);
  spdlog::info("{}", vk2dHostInformation());
  spdlog::info("Window {}x{}, {}x{} pixels", ww, wh, pw, ph);

  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--capture-screenshots") {
      captureScreenshotSuite(window, base);
      vk2dRendererQuit();
      SDL_DestroyWindow(window);
      SDL_Quit();
      pool_shutdown();
      return 0;
    }

  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--test-combat-view") {
      viewCombatReadability(window, base);
      vk2dRendererQuit();
      SDL_DestroyWindow(window);
      SDL_Quit();
      pool_shutdown();
      return 0;
    }

  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--test-encounter-view") {
      viewBiomeEncounter(window, base);
      vk2dRendererQuit();
      SDL_DestroyWindow(window);
      SDL_Quit();
      pool_shutdown();
      return 0;
    }

  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--test-gui-view") {
      viewGui(window, base);
      vk2dRendererQuit();
      SDL_DestroyWindow(window);
      SDL_Quit();
      pool_shutdown();
      return 0;
    }

  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--test-effects-view") {
      viewDeathEffects(window, base);
      vk2dRendererQuit();
      SDL_DestroyWindow(window);
      SDL_Quit();
      pool_shutdown();
      return 0;
    }

  for (int i = 1; i + 3 < argc; i++)
    if (std::string_view(argv[i]) == "--test-biome-view") {
      int x = 0, y = 0;
      unsigned seed = 0;
      auto parse = [](const char *s, auto &value) {
        const char *end = s + std::strlen(s);
        auto result = std::from_chars(s, end, value);
        return result.ec == std::errc{} && result.ptr == end;
      };
      if (!parse(argv[i + 1], x) || !parse(argv[i + 2], y) ||
          !parse(argv[i + 3], seed) ||
          x < -1000000 || x > 1000000 || y < -1000000 || y > 1000000)
        return 1;
      viewBiomeTransition(window, base, x, y, seed);
      vk2dRendererQuit();
      SDL_DestroyWindow(window);
      SDL_Quit();
      pool_shutdown();
      return 0;
    }

  for (int i = 1; i < argc; i++)
    if (std::string_view(argv[i]) == "--test")
      return runTests();
    else if (std::string_view(argv[i]) == "--biome-map" && i + 1 < argc)
      return mapBiomes(argv[i + 1]);

  int code = 0;
  {
    Game game;
    for (int i = 1; i < argc; i++) {
      game.dev |= std::string_view(argv[i]) == "--dev";
      game.bench.enabled |= std::string_view(argv[i]) == "--bench";
      if (std::string_view(argv[i]) == "--arm" && i + 1 < argc)
        game.arm = argv[++i];
      if (std::string_view(argv[i]) == "--hero" && i + 1 < argc)
        game.heroPick = std::atoi(argv[++i]);
      if (std::string_view(argv[i]) == "--biome" && i + 1 < argc)
        game.biomePick = std::atoi(argv[++i]);
    }
    if (game.dev)
      spdlog::info("Dev mode: F5 level up, F6 +30s, F7 god mode, F8 world "
                   "debug, F9 warp to nearest structure, F10 next weather; --arm \"A,B!\" "
                   "starts runs with those upgrades, --biome N starts in biome N");
    if (game.dev) {
      constexpr ouroboros::Ouroboros council;
      int n = int(SDL_GetTicks() % 1000);
      ouroboros::Augury augury;
      spdlog::info("Ouroboros: {} is {}, {} of 4 serpents agree, the augury {}",
                   n, (council->*n) ? "even" : "odd", council.agreement(n),
                   augury.even(n) == (council->*n) ? "concurs" : "dissents");
    }
    if (game.init(window, base)) {
      spdlog::info("Mellangan ready");
      game.run();
    } else {
      code = 1;
    }
    game.shutdown();
  }
  vk2dRendererQuit();
  SDL_DestroyWindow(window);
  SDL_Quit();
  pool_shutdown();
  spdlog::info("Bye");
  return code;
}

#if defined(_WIN32)
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  return main(__argc, __argv);
}
#endif

