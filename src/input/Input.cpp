#include "input/Input.hpp"

#include <algorithm>
#include <cmath>

void Input::handle(const SDL_Event &e, float density) {
  switch (e.type) {
  case SDL_EVENT_GAMEPAD_ADDED:
    SDL_OpenGamepad(e.gdevice.which);
    break;
  case SDL_EVENT_GAMEPAD_REMOVED:
    if (SDL_Gamepad *g = SDL_GetGamepadFromID(e.gdevice.which))
      SDL_CloseGamepad(g);
    break;
  case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
  case SDL_EVENT_GAMEPAD_BUTTON_UP: {
    Key *ks[3];
    int n = bind(SDL_GamepadButton(e.gbutton.button), ks);
    for (int i = 0; i < n; i++)
      ks[i]->toggle(e.gbutton.down);
    pad |= e.gbutton.down;
    break;
  }
  case SDL_EVENT_GAMEPAD_AXIS_MOTION:
    if (e.gaxis.axis < 4 || e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ||
        e.gaxis.axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
      float v = e.gaxis.value / 32767.0f;
      if (e.gaxis.axis < 4)
        axis[e.gaxis.axis] = v;
      else {
        int t = e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? 4 : 5;
        bool now = held[t] ? v > 0.25f : v > 0.5f;
        if (now != held[t])
          held[t] = now, (t == 4 ? campaign : build).toggle(now), pad |= now;
      }
      steer();
    }
    break;
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
    pad = false;
    if (Key *k = bind(e.key.scancode))
      k->toggle(e.type == SDL_EVENT_KEY_DOWN);
    if (e.key.scancode == SDL_SCANCODE_UP || e.key.scancode == SDL_SCANCODE_DOWN)
      (e.key.scancode == SDL_SCANCODE_UP ? older : newer)
          .toggle(e.type == SDL_EVENT_KEY_DOWN);
    if (e.key.scancode == SDL_SCANCODE_LEFT ||
        e.key.scancode == SDL_SCANCODE_RIGHT)
      (e.key.scancode == SDL_SCANCODE_LEFT ? prior : next)
          .toggle(e.type == SDL_EVENT_KEY_DOWN);
    break;
  case SDL_EVENT_TEXT_INPUT:
    pending += e.text.text;
    break;
  case SDL_EVENT_MOUSE_MOTION:
    mouseX = e.motion.x * density, mouseY = e.motion.y * density, moves++;
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    if (e.button.button == SDL_BUTTON_LEFT)
      mouseX = e.button.x * density, mouseY = e.button.y * density, clicks++;
    break;
  case SDL_EVENT_WINDOW_FOCUS_LOST:
    for (Key *k : keys())
      k->release();
    for (float &a : axis)
      a = 0;
    for (bool &h : held)
      h = false;
    stickX = stickY = 0;
    break;
  default:
    break;
  }
}

void Input::tick() {
  for (Key *k : keys())
    k->tick();
  mouseClicked = clicks > 0, mouseMoved = moves > 0;
  typed = std::move(pending);
  pending.clear();
  clicks = moves = 0;
}

std::array<Key *, 36> Input::keys() {
  return {&up,   &down,     &left,    &right,   &confirm,  &escape, &pause,
          &one,  &two,      &three,   &four, &restart, &quit, &map, &codex, &erase, &older, &newer, &prior, &next, &fps,    &fullscreen,
          &mute, &particles, &devLevel, &devTime, &devGod,  &devWorld, &devWarp, &devSky, &loot, &inventory, &campaign, &gather, &build, &options};
}

std::string Input::prompt(std::string_view text) const {
  static constexpr std::string_view SWAP[][2] = {
      {"Enter", "A"},       {"Enter:", "A:"},      {"Esc", "B"},          {"SPACE", "A"},
      {"Space", "A"},       {"Tab", "Back"},       {"Arrows", "D-pad"},   {"WASD/Arrows", "Stick"},
      {"Up/Down", "D-pad"}, {"Left/Right", "D-pad"}, {"Backspace", "L3"}, {"I", "Y"},
      {"K", "LT"},          {"E", "X"},            {"R", "X"},            {"B", "RT"},
      {"C", "Y"},           {"O", "RB"},           {"Q", "Back"},         {"P", "Start"}};
  if (!pad)
    return std::string(text);
  std::string out;
  for (size_t at = 0; at < text.size();) {
    size_t end = text.find(' ', at);
    if (end == at) {
      out += ' ', at++;
      continue;
    }
    std::string_view word = text.substr(at, end == std::string_view::npos ? end : end - at);
    std::string_view swap = word;
    for (const auto &s : SWAP)
      if (word == s[0])
        swap = s[1];
    out += swap, at += word.size();
  }
  return out;
}

void Input::steer() {
  float x = axis[SDL_GAMEPAD_AXIS_LEFTX], y = axis[SDL_GAMEPAD_AXIS_LEFTY],
        m = std::sqrt(x * x + y * y);
  constexpr float DEAD = 0.25f;
  float k = m > DEAD ? std::min(1.0f, (m - DEAD) / (1 - DEAD)) / m : 0;
  stickX = x * k, stickY = y * k;
  if (m > DEAD)
    pad = true;
  Key *dirs[4] = {&left, &right, &up, &down};
  Key *steps[4] = {&prior, &next, &older, &newer};
  float along[4] = {-stickX, stickX, -stickY, stickY};
  for (int i = 0; i < 4; i++) {
    bool now = held[i] ? along[i] > 0.3f : along[i] > 0.6f;
    if (now != held[i])
      held[i] = now, dirs[i]->toggle(now), steps[i]->toggle(now);
  }
}

int Input::bind(SDL_GamepadButton button, Key **out) {
  switch (button) {
  case SDL_GAMEPAD_BUTTON_SOUTH:
    return out[0] = &confirm, 1;
  case SDL_GAMEPAD_BUTTON_EAST:
    return out[0] = &escape, 1;
  case SDL_GAMEPAD_BUTTON_WEST:
    return out[0] = &gather, out[1] = &restart, 2;
  case SDL_GAMEPAD_BUTTON_NORTH:
    return out[0] = &inventory, out[1] = &codex, 2;
  case SDL_GAMEPAD_BUTTON_START:
    return out[0] = &pause, 1;
  case SDL_GAMEPAD_BUTTON_BACK:
    return out[0] = &map, out[1] = &quit, 2;
  case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
    return out[0] = &loot, 1;
  case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
    return out[0] = &options, 1;
  case SDL_GAMEPAD_BUTTON_LEFT_STICK:
    return out[0] = &erase, 1;
  case SDL_GAMEPAD_BUTTON_DPAD_UP:
    return out[0] = &up, out[1] = &older, 2;
  case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
    return out[0] = &down, out[1] = &newer, 2;
  case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
    return out[0] = &left, out[1] = &prior, 2;
  case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
    return out[0] = &right, out[1] = &next, 2;
  default:
    return 0;
  }
}

Key *Input::bind(SDL_Scancode sc) {
  switch (sc) {
  case SDL_SCANCODE_W:
  case SDL_SCANCODE_UP:
    return &up;
  case SDL_SCANCODE_S:
  case SDL_SCANCODE_DOWN:
    return &down;
  case SDL_SCANCODE_A:
  case SDL_SCANCODE_LEFT:
    return &left;
  case SDL_SCANCODE_D:
  case SDL_SCANCODE_RIGHT:
    return &right;
  case SDL_SCANCODE_RETURN:
  case SDL_SCANCODE_KP_ENTER:
  case SDL_SCANCODE_SPACE:
    return &confirm;
  case SDL_SCANCODE_ESCAPE:
    return &escape;
  case SDL_SCANCODE_P:
    return &pause;
  case SDL_SCANCODE_1:
    return &one;
  case SDL_SCANCODE_2:
    return &two;
  case SDL_SCANCODE_3:
    return &three;
  case SDL_SCANCODE_4:
    return &four;
  case SDL_SCANCODE_R:
    return &restart;
  case SDL_SCANCODE_Q:
    return &quit;
  case SDL_SCANCODE_TAB:
    return &map;
  case SDL_SCANCODE_C:
    return &codex;
  case SDL_SCANCODE_BACKSPACE:
    return &erase;
  case SDL_SCANCODE_F3:
    return &fps;
  case SDL_SCANCODE_F11:
    return &fullscreen;
  case SDL_SCANCODE_M:
    return &mute;
  case SDL_SCANCODE_V:
    return &particles;
  case SDL_SCANCODE_F5:
    return &devLevel;
  case SDL_SCANCODE_F6:
    return &devTime;
  case SDL_SCANCODE_F7:
    return &devGod;
  case SDL_SCANCODE_F8:
    return &devWorld;
  case SDL_SCANCODE_F9:
    return &devWarp;
  case SDL_SCANCODE_F10:
    return &devSky;
  case SDL_SCANCODE_I:
    return &inventory;
  case SDL_SCANCODE_K:
    return &campaign;
  case SDL_SCANCODE_E:
    return &gather;
  case SDL_SCANCODE_B:
    return &build;
  case SDL_SCANCODE_O:
    return &options;
  case SDL_SCANCODE_LALT:
  case SDL_SCANCODE_RALT:
    return &loot;
  default:
    return nullptr;
  }
}
