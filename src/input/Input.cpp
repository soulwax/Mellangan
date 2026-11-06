#include "input/Input.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>

const Input::Binding Input::BINDINGS[] = {
    {"MOVE UP", nullptr, &Input::up, SDL_SCANCODE_W},
    {"MOVE DOWN", nullptr, &Input::down, SDL_SCANCODE_S},
    {"MOVE LEFT", nullptr, &Input::left, SDL_SCANCODE_A},
    {"MOVE RIGHT", nullptr, &Input::right, SDL_SCANCODE_D},
    {"CLASS SKILL", "SPACE", &Input::confirm, SDL_SCANCODE_SPACE},
    {"PAUSE", "P", &Input::pause, SDL_SCANCODE_P},
    {"MAP", "Tab", &Input::map, SDL_SCANCODE_TAB},
    {"INVENTORY", "I", &Input::inventory, SDL_SCANCODE_I},
    {"GATHER", "E", &Input::gather, SDL_SCANCODE_E},
    {"BUILD", "B", &Input::build, SDL_SCANCODE_B},
    {"SHOW LOOT", nullptr, &Input::loot, SDL_SCANCODE_LALT},
    {"CODEX", "C", &Input::codex, SDL_SCANCODE_C},
    {"SETTINGS", "O", &Input::options, SDL_SCANCODE_O},
    {"SKILL TREE", "K", &Input::campaign, SDL_SCANCODE_K},
    {"RETRY", "R", &Input::restart, SDL_SCANCODE_R},
    {"MUTE", "M", &Input::mute, SDL_SCANCODE_M},
    {"PARTICLES", "V", &Input::particles, SDL_SCANCODE_V}};
static_assert(sizeof Input::BINDINGS / sizeof *Input::BINDINGS == Input::BINDING_COUNT);

const Input::PadBinding Input::PAD_BINDINGS[] = {
    {"GATHER / RETRY", &Input::gather, &Input::restart, SDL_GAMEPAD_BUTTON_WEST, " E R "},
    {"INVENTORY / CODEX", &Input::inventory, &Input::codex, SDL_GAMEPAD_BUTTON_NORTH, " I C "},
    {"MAP / QUIT", &Input::map, &Input::quit, SDL_GAMEPAD_BUTTON_BACK, " Tab Q "},
    {"SHOW LOOT", &Input::loot, nullptr, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, " Alt "},
    {"SETTINGS", &Input::options, nullptr, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, " O "},
    {"DROP ITEM", &Input::erase, nullptr, SDL_GAMEPAD_BUTTON_LEFT_STICK, " Backspace "},
    {"SKILL TREE", &Input::campaign, nullptr, Input::LT, " K "},
    {"BUILD", &Input::build, nullptr, Input::RT, " B "}};
static_assert(sizeof Input::PAD_BINDINGS / sizeof *Input::PAD_BINDINGS == Input::PAD_COUNT);

namespace {
struct PadButton {
  int id;
  const char *name;
};

constexpr PadButton PAD_BUTTONS[] = {
    {SDL_GAMEPAD_BUTTON_WEST, "X"},           {SDL_GAMEPAD_BUTTON_NORTH, "Y"},
    {SDL_GAMEPAD_BUTTON_BACK, "Back"},        {SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, "LB"},
    {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, "RB"}, {SDL_GAMEPAD_BUTTON_LEFT_STICK, "L3"},
    {SDL_GAMEPAD_BUTTON_RIGHT_STICK, "R3"},   {Input::LT, "LT"},
    {Input::RT, "RT"},                        {SDL_GAMEPAD_BUTTON_MISC1, "Share"},
    {SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1, "P1"}, {SDL_GAMEPAD_BUTTON_LEFT_PADDLE1, "P2"},
    {SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2, "P3"}, {SDL_GAMEPAD_BUTTON_LEFT_PADDLE2, "P4"}};
} // namespace

Input::Input() { resetKeys(); }

void Input::resetKeys() {
  for (int i = 0; i < BINDING_COUNT; i++)
    keymap[i] = BINDINGS[i].fallback;
  for (int i = 0; i < PAD_COUNT; i++)
    padmap[i] = PAD_BINDINGS[i].fallback;
}

const char *Input::padName(int button) {
  for (const PadButton &b : PAD_BUTTONS)
    if (b.id == button)
      return b.name;
  return nullptr;
}

bool Input::rebindPad(int b, int button) {
  if (b < 0 || b >= PAD_COUNT || !padName(button))
    return false;
  for (int i = 0; i < PAD_COUNT; i++)
    if (padmap[i] == button)
      padmap[i] = padmap[b];
  padmap[b] = button;
  for (Key *k : keys())
    k->release();
  return true;
}

bool Input::rebind(int b, SDL_Scancode sc) {
  if (b < 0 || b >= BINDING_COUNT || sc == SDL_SCANCODE_UNKNOWN || fixed(sc))
    return false;
  for (int i = 0; i < BINDING_COUNT; i++)
    if (keymap[i] == sc)
      keymap[i] = keymap[b];
  keymap[b] = sc;
  for (Key *k : keys())
    k->release();
  return true;
}

std::string Input::keyName(SDL_Scancode sc) {
  std::string name = SDL_GetScancodeName(sc);
  for (char &c : name)
    c = char(std::toupper(uint8_t(c)));
  return name.empty() ? "?" : name;
}

std::string Input::saveKeys() const {
  std::string out;
  for (int i = 0; i < BINDING_COUNT; i++) {
    std::string key = BINDINGS[i].name;
    std::replace(key.begin(), key.end(), ' ', '_');
    out += key + " " + std::to_string(int(keymap[i])) + "\n";
  }
  for (int i = 0; i < PAD_COUNT; i++) {
    std::string key = std::string("PAD ") + PAD_BINDINGS[i].name;
    std::replace(key.begin(), key.end(), ' ', '_');
    out += key + " " + std::to_string(padmap[i]) + "\n";
  }
  return out;
}

void Input::parseKeys(std::string_view text) {
  std::string s(text);
  for (const char *at = s.c_str(); *at;) {
    char key[32] = {};
    int sc = 0;
    if (std::sscanf(at, "%31s %d", key, &sc) == 2)
      for (int i = 0; i < BINDING_COUNT; i++) {
        std::string name = BINDINGS[i].name;
        std::replace(name.begin(), name.end(), ' ', '_');
        if (name == key && sc > 0 && sc < SDL_SCANCODE_COUNT)
          rebind(i, SDL_Scancode(sc));
      }
    if (std::sscanf(at, "%31s %d", key, &sc) == 2)
      for (int i = 0; i < PAD_COUNT; i++) {
        std::string name = std::string("PAD ") + PAD_BINDINGS[i].name;
        std::replace(name.begin(), name.end(), ' ', '_');
        if (name == key)
          rebindPad(i, sc);
      }
    const char *end = std::strchr(at, '\n');
    at = end ? end + 1 : at + std::strlen(at);
  }
}

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
    if (listening >= 0) {
      listening = e.gbutton.down ? -1 : listening;
      break;
    }
    press(e.gbutton.button, e.gbutton.down);
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
          held[t] = now, press(t == 4 ? LT : RT, now);
      }
      steer();
    }
    break;
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
    pad = false;
    if (padListening >= 0) {
      padListening = -1;
      break;
    }
    if (listening >= 0) {
      if (e.type == SDL_EVENT_KEY_DOWN && !e.key.repeat) {
        if (e.key.scancode != SDL_SCANCODE_ESCAPE)
          rebind(listening, e.key.scancode);
        listening = -1;
      }
      break;
    }
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
      {"Up/Down", "D-pad"}, {"Left/Right", "D-pad"}, {"P", "Start"}};
  std::string out;
  for (size_t at = 0; at < text.size();) {
    size_t end = text.find(' ', at);
    if (end == at) {
      out += ' ', at++;
      continue;
    }
    std::string_view word = text.substr(at, end == std::string_view::npos ? end : end - at);
    std::string swap(word);
    if (pad) {
      for (const auto &s : SWAP)
        if (word == s[0])
          swap = s[1];
      std::string spaced = " " + std::string(word) + " ";
      for (int i = 0; i < PAD_COUNT; i++)
        if (std::strstr(PAD_BINDINGS[i].words, spaced.c_str()))
          swap = padName(padmap[i]);
    } else {
      for (int i = 0; i < BINDING_COUNT; i++)
        if (BINDINGS[i].word && word == BINDINGS[i].word && keymap[i] != BINDINGS[i].fallback)
          swap = keyName(keymap[i]);
    }
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

void Input::press(int button, bool pressed) {
  if (padListening >= 0 && pressed) {
    if (button != SDL_GAMEPAD_BUTTON_EAST)
      rebindPad(padListening, button);
    padListening = -1;
    return;
  }
  Key *ks[3];
  int n = bind(button, ks);
  for (int i = 0; i < n; i++)
    ks[i]->toggle(pressed);
  pad |= pressed;
}

int Input::bind(int button, Key **out) {
  for (int i = 0; i < PAD_COUNT; i++)
    if (padmap[i] == button) {
      out[0] = &(this->*PAD_BINDINGS[i].first);
      if (!PAD_BINDINGS[i].second)
        return 1;
      out[1] = &(this->*PAD_BINDINGS[i].second);
      return 2;
    }
  switch (button) {
  case SDL_GAMEPAD_BUTTON_SOUTH:
    return out[0] = &confirm, 1;
  case SDL_GAMEPAD_BUTTON_EAST:
    return out[0] = &escape, 1;
  case SDL_GAMEPAD_BUTTON_START:
    return out[0] = &pause, 1;
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
  for (int i = 0; i < BINDING_COUNT; i++)
    if (keymap[i] == sc)
      return &(this->*BINDINGS[i].key);
  return fixed(sc);
}

Key *Input::fixed(SDL_Scancode sc) {
  switch (sc) {
  case SDL_SCANCODE_UP:
    return &up;
  case SDL_SCANCODE_DOWN:
    return &down;
  case SDL_SCANCODE_LEFT:
    return &left;
  case SDL_SCANCODE_RIGHT:
    return &right;
  case SDL_SCANCODE_RETURN:
  case SDL_SCANCODE_KP_ENTER:
    return &confirm;
  case SDL_SCANCODE_ESCAPE:
    return &escape;
  case SDL_SCANCODE_1:
    return &one;
  case SDL_SCANCODE_2:
    return &two;
  case SDL_SCANCODE_3:
    return &three;
  case SDL_SCANCODE_4:
    return &four;
  case SDL_SCANCODE_Q:
    return &quit;
  case SDL_SCANCODE_BACKSPACE:
    return &erase;
  case SDL_SCANCODE_F3:
    return &fps;
  case SDL_SCANCODE_F11:
    return &fullscreen;
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
  case SDL_SCANCODE_RALT:
    return &loot;
  default:
    return nullptr;
  }
}
