#include "input/Input.hpp"

void Input::handle(const SDL_Event &e, float density) {
  switch (e.type) {
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
    if (Key *k = bind(e.key.scancode))
      k->toggle(e.type == SDL_EVENT_KEY_DOWN);
    if (e.key.scancode == SDL_SCANCODE_UP || e.key.scancode == SDL_SCANCODE_DOWN)
      (e.key.scancode == SDL_SCANCODE_UP ? older : newer)
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

std::array<Key *, 25> Input::keys() {
  return {&up,   &down,     &left,    &right,   &confirm,  &escape, &pause,
          &one,  &two,      &three,   &four, &restart, &quit, &map, &erase, &older, &newer, &fps,    &fullscreen,
          &mute, &devLevel, &devTime, &devGod,  &devWorld, &devWarp};
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
  case SDL_SCANCODE_BACKSPACE:
    return &erase;
  case SDL_SCANCODE_F3:
    return &fps;
  case SDL_SCANCODE_F11:
    return &fullscreen;
  case SDL_SCANCODE_M:
    return &mute;
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
  default:
    return nullptr;
  }
}
