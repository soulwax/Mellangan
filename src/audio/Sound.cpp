#include "audio/Sound.hpp"

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#include <spdlog/spdlog.h>

#include <array>
#include <cstdlib>

#include "Math.hpp"

namespace {
constexpr SDL_AudioSpec SPEC{SDL_AUDIO_S16, 1, 44100};
constexpr float MUSIC_VOLUME = 0.28f;

SDL_AudioDeviceID device = 0;
std::array<SDL_AudioStream *, 32> voices{};
std::array<Uint64, 32> voiceStarted{};
SDL_AudioStream *musicStream = nullptr;
short *musicData = nullptr;
int musicLength = 0;
bool muted = false;

void loadMusic(const std::string &path) {
    int channels = 0, rate = 0;
    int frames = stb_vorbis_decode_filename(path.c_str(), &channels, &rate, &musicData);
    if (frames <= 0) {
        spdlog::warn("Can't decode {}", path);
        return;
    }
    musicLength = frames * channels * int(sizeof(short));
    SDL_AudioSpec spec{SDL_AUDIO_S16, channels, rate};
    musicStream = SDL_CreateAudioStream(&spec, nullptr);
    SDL_BindAudioStream(device, musicStream);
    SDL_SetAudioStreamGain(musicStream, MUSIC_VOLUME);
    spdlog::info("Music: {} ({:.0f} s, {} Hz, {} ch)", path, frames / double(rate), rate, channels);
}
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
Sound Sound::thunder{"thunder", 0.08f};
Sound Sound::explosion{"explosion", 0.08f};
Sound Sound::bow{"bow", 0.05f};
Sound Sound::scythe{"scythe", 0.1f};
Sound Sound::magnet{"magnet", 0};
Sound Sound::nova{"nova", 0};
Sound Sound::barrel{"barrel", 0.05f};
Sound Sound::orb{"orb", 0.1f};

namespace {
Sound *const ALL[] = {&Sound::cast,  &Sound::hit,     &Sound::kill,    &Sound::slash,   &Sound::axe,
                      &Sound::gem,   &Sound::levelUp, &Sound::hurt,    &Sound::death,   &Sound::chest,
                      &Sound::potion, &Sound::select, &Sound::confirm, &Sound::boss,    &Sound::horde,
                      &Sound::thunder, &Sound::explosion, &Sound::bow, &Sound::scythe, &Sound::magnet,
                      &Sound::nova,  &Sound::barrel,  &Sound::orb};
} // namespace

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
    SDL_AudioSpec deviceSpec;
    int frames = 0;
    if (SDL_GetAudioDeviceFormat(device, &deviceSpec, &frames))
        spdlog::info("Audio: {} Hz, {} ch, {} frame buffer", deviceSpec.freq, deviceSpec.channels, frames);
    for (auto &v : voices) v = SDL_CreateAudioStream(&SPEC, nullptr), SDL_BindAudioStream(device, v);

    for (Sound *s : ALL) s->load(base + "res/sfx/" + s->file + ".wav");

    loadMusic(base + "res/music/night.ogg");
    SDL_ResumeAudioDevice(device);
    return true;
}

void Sound::shutdown() {
    if (!device) return;
    for (auto *v : voices) SDL_DestroyAudioStream(v);
    SDL_DestroyAudioStream(musicStream);
    SDL_CloseAudioDevice(device);
    for (Sound *s : ALL) SDL_free(s->data), s->data = nullptr;
    std::free(musicData);
    musicData = nullptr;
    device = 0;
}

void Sound::tick() {
    if (musicStream && SDL_GetAudioStreamQueued(musicStream) < musicLength)
        SDL_PutAudioStreamData(musicStream, musicData, musicLength);
}

void Sound::toggleMute() {
    muted = !muted;
    if (device) SDL_SetAudioDeviceGain(device, muted ? 0.0f : 1.0f);
}

void Sound::play(float volume, float pitchJitter) {
    if (!device || !data) return;
    Uint64 now = SDL_GetTicks();
    if (now - last < Uint64(gap * 1000)) return;
    size_t pick = 0;
    for (size_t i = 0; i < voices.size(); i++) {
        if (SDL_GetAudioStreamQueued(voices[i]) == 0) {
            pick = i;
            break;
        }
        if (voiceStarted[i] < voiceStarted[pick]) pick = i;
    }
    SDL_AudioStream *v = voices[pick];
    SDL_ClearAudioStream(v);
    SDL_SetAudioStreamGain(v, volume);
    SDL_SetAudioStreamFrequencyRatio(v, 1 + rng.range(-pitchJitter, pitchJitter));
    SDL_PutAudioStreamData(v, data, int(length));
    SDL_FlushAudioStream(v);
    voiceStarted[pick] = last = now;
}
