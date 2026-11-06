#include "menu/IntroMenu.hpp"

#include <cstdio>

#include <pl_mpeg.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <filesystem>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "menu/TitleMenu.hpp"

namespace {
const char *const VIDEO = "res/videos/moth_studios.mpg", *const AUDIO = "res/videos/moth_studios.ogg";
}

bool IntroMenu::ready(const std::string &base) {
  std::error_code ec;
  return std::filesystem::exists(base + VIDEO, ec) && std::filesystem::exists(base + AUDIO, ec);
}

IntroMenu::IntroMenu(Game &game) : Menu(game) {
  video = plm_create_with_filename((game.base + VIDEO).c_str());
  if (video) {
    plm_set_audio_enabled(video, 0);
    w = plm_get_width(video), h = plm_get_height(video), length = plm_get_duration(video);
    spdlog::info("Intro: {}x{}, {:.2f} s", w, h, length);
    rgba.assign(size_t(w) * size_t(h) * 4, 255);
    image = vk2dImageFromPixels(vk2dRendererGetDevice(), rgba.data(), w, h, true);
    frame = image ? vk2dTextureLoadFromImage(image) : nullptr;
  }
  if (!frame || length <= 0) {
    length = 0;
    return;
  }
  Sound::clip(game.base + AUDIO);
  decode();
}

IntroMenu::~IntroMenu() {
  Sound::endClip();
  vk2dRendererWait();
  if (frame)
    vk2dTextureFree(frame);
  if (image)
    vk2dImageFree(image);
  if (video)
    plm_destroy(video);
}

double IntroMenu::now() const {
  return std::max(Sound::clipTime(), double(t));
}

float IntroMenu::fade() const {
  double end = ending >= 0 ? std::min(double(ending) + FADE, length) : length;
  return std::clamp(float(end - now()) / FADE, 0.0f, 1.0f);
}

void IntroMenu::decode() {
  plm_frame_t *shown = nullptr;
  for (double at = now(); video && next <= at;) {
    plm_frame_t *f = plm_decode_video(video);
    if (!f) {
      next = length + 1;
      break;
    }
    shown = f, next = f->time + 1 / plm_get_framerate(video);
  }
  if (shown)
    plm_frame_to_rgba(shown, rgba.data(), w * 4), vk2dTextureUpdate(frame, rgba.data());
}

void IntroMenu::finish() {
  auto title = std::make_unique<TitleMenu>(game);
  title->veil = FADE;
  game.setMenu(std::move(title));
}

void IntroMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (ending < 0 && t > 0.1f && (in.any || in.mouseClicked))
    ending = float(now());
  if (fade() <= 0)
    return finish();
  decode();
  Sound::clipGain(fade() * fade());
}

void IntroMenu::render(Screen &s) {
  s.rect(0, 0, s.w, s.h, col::black);
  if (!frame || fade() <= 0)
    return;
  float k = std::min(s.w / float(w), s.h / float(h)), dw = w * k, dh = h * k, f = std::pow(fade(), 2.2f);
  s.image(frame, std::floor((s.w - dw) / 2), std::floor((s.h - dh) / 2), dw, dh, {f, f, f, 1});
}
