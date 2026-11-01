#pragma once

class Level;

class Thunder {
public:
  float flash = 0;

  void tick();
  void storm(Level &level);

private:
  float in = 0, volume = 0, pitch = 1, flicker = 0, strikeTime = 4;
};
