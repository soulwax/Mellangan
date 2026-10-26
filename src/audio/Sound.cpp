#include "audio/Sound.hpp"

#include <spdlog/spdlog.h>

#include <array>

#include "Math.hpp"

namespace {
constexpr SDL_AudioSpec SPEC{SDL_AUDIO_S16, 1, 44100};
constexpr float MUSIC_VOLUME = 0.35f;

SDL_AudioDeviceID device = 0;
std::array<SDL_AudioStream *, 32> voices{};
SDL_AudioStream *musicStream = nullptr;
Uint8 *musicData = nullptr;
Uint32 musicLength = 0;
bool muted = false;
} // namespace

Sound Sound::cast{"cast", 0.05f};
Sound Sound::hit{"hit", 0.04f};
Sound Sound::kill{"kill", 0.03f};
Sound Sound::slash{"slash", 0.05f};
Sound Sound::axe{"axe", 0.05f};
Sound Sound::gem{"gem", 0.045f};
Sound Sound::levelUp{"levelup", 0};
Sound Sound::hurt{"hurt", 0.1f};
Sound Sound::death{"death", 0};
Sound Sound::chest{"chest", 0};
Sound Sound::potion{"potion", 0};
Sound Sound::select{"select", 0};
Sound Sound::confirm{"confirm", 0};
Sound Sound::boss{"boss", 0};
Sound Sound::horde{"horde", 0};

bool Sound::load(const std::string &path) {
    SDL_AudioSpec spec;
    if (!SDL_LoadWAV(path.c_str(), &spec, &data, &length)) {
        spdlog::warn("Can't load {}: {}", path, SDL_GetError());
        return false;
    }
    if (spec.format != SPEC.format || spec.channels != SPEC.channels || spec.freq != SPEC.freq) {
        spdlog::warn("{} is not mono 44.1 kHz s16; rerun tools/make_audio.py", path);
        SDL_free(data), data = nullptr, length = 0;
        return false;
    }
    return true;
}

bool Sound::init(const std::string &base) {
    device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &SPEC);
    if (!device) {
        spdlog::warn("No audio device: {}", SDL_GetError());
        return false;
    }
    for (auto &v : voices) v = SDL_CreateAudioStream(&SPEC, &SPEC), SDL_BindAudioStream(device, v);
    musicStream = SDL_CreateAudioStream(&SPEC, &SPEC);
    SDL_BindAudioStream(device, musicStream);
    SDL_SetAudioStreamGain(musicStream, MUSIC_VOLUME);

    for (Sound *s : {&cast, &hit, &kill, &slash, &axe, &gem, &levelUp, &hurt, &death, &chest, &potion, &select,
                     &confirm, &boss, &horde})
        s->load(base + "res/sfx/" + s->file + ".wav");

    SDL_AudioSpec spec;
    if (!SDL_LoadWAV((base + "res/music/night.wav").c_str(), &spec, &musicData, &musicLength))
        spdlog::warn("No music: {}", SDL_GetError());
    SDL_ResumeAudioDevice(device);
    return true;
}

void Sound::shutdown() {
    if (!device) return;
    for (auto *v : voices) SDL_DestroyAudioStream(v);
    SDL_DestroyAudioStream(musicStream);
    SDL_CloseAudioDevice(device);
    for (Sound *s : {&cast, &hit, &kill, &slash, &axe, &gem, &levelUp, &hurt, &death, &chest, &potion, &select,
                     &confirm, &boss, &horde})
        SDL_free(s->data), s->data = nullptr;
    SDL_free(musicData);
    device = 0;
}

void Sound::tick() {
    // Keep at least one full copy of the loop queued so it never runs dry.
    if (musicStream && musicData && SDL_GetAudioStreamQueued(musicStream) < int(musicLength))
        SDL_PutAudioStreamData(musicStream, musicData, int(musicLength));
}

void Sound::toggleMute() {
    muted = !muted;
    if (device) SDL_SetAudioDeviceGain(device, muted ? 0.0f : 1.0f);
}

void Sound::play(float volume, float pitchJitter) {
    if (!device || !data) return;
    Uint64 now = SDL_GetTicks();
    if (now - last < Uint64(gap * 1000)) return;
    for (auto *v : voices) {
        if (SDL_GetAudioStreamQueued(v) > 0) continue;
        last = now;
        SDL_SetAudioStreamGain(v, volume);
        SDL_SetAudioStreamFrequencyRatio(v, 1 + rng.range(-pitchJitter, pitchJitter));
        SDL_PutAudioStreamData(v, data, int(length));
        return;
    }
}
