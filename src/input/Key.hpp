#pragma once

class Key {
public:
  bool down = false, clicked = false;

  void toggle(bool pressed) {
    if (pressed && !down)
      presses++;
    down = pressed;
  }
  void tick() {
    clicked = presses > 0;
    presses = 0;
  }
  void release() { down = false, presses = 0; }

private:
  int presses = 0;
};
