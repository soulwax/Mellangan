#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <SDL3/SDL.h>

#include <cmath>
#include <iostream>
#include <string>

#include "input/Input.hpp"

namespace {
void button(Input &in, SDL_GamepadButton b, bool down) {
  SDL_Event e{};
  e.type = down ? SDL_EVENT_GAMEPAD_BUTTON_DOWN : SDL_EVENT_GAMEPAD_BUTTON_UP;
  e.gbutton.button = Uint8(b), e.gbutton.down = down;
  in.handle(e, 1);
}

void axis(Input &in, SDL_GamepadAxis a, float v) {
  SDL_Event e{};
  e.type = SDL_EVENT_GAMEPAD_AXIS_MOTION;
  e.gaxis.axis = Uint8(a), e.gaxis.value = Sint16(v * 32767);
  in.handle(e, 1);
}
} // namespace

void testGamepad() {
  Input in;
  button(in, SDL_GAMEPAD_BUTTON_SOUTH, true);
  in.tick();
  assert(in.confirm.down && in.confirm.clicked && in.pad);
  button(in, SDL_GAMEPAD_BUTTON_SOUTH, false);
  in.tick();
  assert(!in.confirm.down && !in.confirm.clicked);

  button(in, SDL_GAMEPAD_BUTTON_NORTH, true);
  in.tick();
  assert(in.inventory.clicked && in.codex.clicked);
  button(in, SDL_GAMEPAD_BUTTON_NORTH, false);

  axis(in, SDL_GAMEPAD_AXIS_LEFTX, 0.2f);
  in.tick();
  assert(in.stickX == 0 && !in.right.down);
  axis(in, SDL_GAMEPAD_AXIS_LEFTX, 0.5f);
  in.tick();
  assert(in.stickX > 0.3f && in.stickX < 0.4f && !in.right.down);
  axis(in, SDL_GAMEPAD_AXIS_LEFTX, 1);
  in.tick();
  assert(std::abs(in.stickX - 1) < 0.01f && in.right.down && in.right.clicked && in.next.clicked);
  axis(in, SDL_GAMEPAD_AXIS_LEFTX, 0.5f);
  in.tick();
  assert(in.right.down && !in.right.clicked);
  axis(in, SDL_GAMEPAD_AXIS_LEFTX, 0.3f);
  in.tick();
  assert(!in.right.down && in.stickX < 0.1f);

  axis(in, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.9f);
  in.tick();
  assert(in.build.clicked);
  axis(in, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0);
  in.tick();
  assert(!in.build.down);

  assert(in.pad);
  assert(in.prompt("Arrows or mouse pick   Enter equip   Backspace drop   I close") ==
         "D-pad or mouse pick   A equip   L3 drop   Y close");
  assert(in.prompt("B  raise a watchtower   Esc close") == "RT  raise a watchtower   B close");
  assert(in.prompt("Enter: learn") == "A: learn" && in.prompt("SPACE") == "A");

  SDL_Event key{};
  key.type = SDL_EVENT_KEY_DOWN, key.key.scancode = SDL_SCANCODE_W, key.key.down = true;
  in.handle(key, 1);
  in.tick();
  assert(!in.pad && in.up.clicked);
  assert(in.prompt("Enter equip   I close") == "Enter equip   I close");
  std::cout << "[TEST] Gamepad: buttons reach their keys, the stick has a dead zone, partial speed "
               "and a press/release gap for menus, triggers act as buttons, keys switch prompts "
               "back: PASS"
            << std::endl;
}

void testRebind() {
  Input in;
  auto press = [&](SDL_Scancode sc, bool down) {
    SDL_Event e{};
    e.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    e.key.scancode = sc, e.key.down = down;
    in.handle(e, 1);
  };
  int inventory = 7, gather = 8;
  assert(std::string(Input::BINDINGS[inventory].name) == "INVENTORY" &&
         std::string(Input::BINDINGS[gather].name) == "GATHER");
  in.listening = inventory;
  press(SDL_SCANCODE_G, true);
  press(SDL_SCANCODE_G, false);
  assert(in.listening == -1 && in.keymap[inventory] == SDL_SCANCODE_G);
  press(SDL_SCANCODE_G, true);
  in.tick();
  assert(in.inventory.clicked);
  press(SDL_SCANCODE_G, false);
  press(SDL_SCANCODE_I, true);
  in.tick();
  assert(!in.inventory.down);
  press(SDL_SCANCODE_I, false);
  assert(in.prompt("Enter equip   I close") == "Enter equip   G close");
  assert(in.rebind(inventory, SDL_SCANCODE_E) && in.keymap[gather] == SDL_SCANCODE_G);
  assert(!in.rebind(inventory, SDL_SCANCODE_ESCAPE) && !in.rebind(inventory, SDL_SCANCODE_UP));
  in.listening = gather;
  press(SDL_SCANCODE_ESCAPE, true);
  assert(in.listening == -1 && in.keymap[gather] == SDL_SCANCODE_G);
  in.tick();
  assert(!in.escape.clicked);
  press(SDL_SCANCODE_ESCAPE, false);
  std::string saved = in.saveKeys();
  Input other;
  other.parseKeys(saved);
  for (int i = 0; i < Input::BINDING_COUNT; i++)
    assert(other.keymap[i] == in.keymap[i]);
  other.resetKeys();
  assert(other.keymap[inventory] == SDL_SCANCODE_I && other.prompt("I close") == "I close");
  std::cout << "[TEST] Rebinding: a captured key drives its action and frees the old one, taken keys "
               "swap, fixed keys are refused, Esc cancels, hints name the new key, keys.txt round-trips: PASS"
            << std::endl;
}
