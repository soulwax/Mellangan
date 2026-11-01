#pragma once

#include <SDL3/SDL.h>

#include "hero/Hero.hpp"
#include "menu/Menu.hpp"

class Upgrade;

class ClassMenu : public Menu {
public:
  explicit ClassMenu(Game &game);
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }

private:
  int pick = 0;
  float since = 0;
  float grow[HERO_COUNT]{};

  struct Pose {
    float cycle, wind, strike, recover, lunge;
  };

  SDL_FRect slot(int i) const;
  Pose pose() const;
  void renderHero(Screen &s, int i, const Pose &p, float in) const;
  void renderAura(Screen &s, const Hero &k, float cx, float foot, float height,
                  const Pose &p, float in) const;
  void renderSwing(Screen &s, const Hero &k, const Upgrade &up, float hx, float hy,
                   float scale, const Pose &p, float in) const;
  void choose(int i);
  void begin();
};
