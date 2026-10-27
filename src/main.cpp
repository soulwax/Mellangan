
#include <SDL3/SDL.h>
#include <VK2D/VK2D.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "Crash.hpp"
#include "Game.hpp"
#include "Version.hpp"

static void initLogging(const std::string &base) {
    std::vector<spdlog::sink_ptr> sinks{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
    try {
        sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(base + "mellangan.log", true));
    } catch (const spdlog::spdlog_ex &) {
    }
    auto log = std::make_shared<spdlog::logger>("mellangan", sinks.begin(), sinks.end());
    log->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    log->flush_on(spdlog::level::info);
#ifndef NDEBUG
    log->set_level(spdlog::level::debug);
#endif
    spdlog::set_default_logger(log);
}

int main(int argc, char **argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) return 1;
    const char *basePath = SDL_GetBasePath();
    std::string base = basePath ? basePath : "";
    initLogging(base);
    installCrashHandler();
    spdlog::info("Mellangan v{}", MELLANGAN_VERSION);

    SDL_Window *window = SDL_CreateWindow("Mellangan", Screen::W * 3, Screen::H * 3, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        spdlog::critical("SDL_CreateWindow: {}", SDL_GetError());
        return 1;
    }

    VK2DRendererConfig config{VK2D_MSAA_1X, VK2D_SCREEN_MODE_VSYNC, VK2D_FILTER_TYPE_NEAREST};
    VK2DStartupOptions options{};
#ifndef NDEBUG
    options.enableDebug = true;
#endif
    if (vk2dRendererInit(window, config, &options) != VK2D_SUCCESS) {
        spdlog::critical("Vulkan2D init: {}", vk2dStatusMessage());
        return 1;
    }

    int code = 0;
    {
        Game game;
        for (int i = 1; i < argc; i++) {
            game.dev |= std::string_view(argv[i]) == "--dev";
            game.bench.enabled |= std::string_view(argv[i]) == "--bench";
        }
        if (game.dev) spdlog::info("Dev mode: F5 level up, F6 +30s, F7 god mode, F8 world debug, F9 warp to nearest structure");
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
    spdlog::info("Bye");
    return code;
}
