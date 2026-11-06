#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "menu/Menu.hpp"

struct plm_t;

class IntroMenu : public Menu {
public:
  static constexpr float FADE = 0.5f;

  static bool ready(const std::string &base);
  explicit IntroMenu(Game &game);
  ~IntroMenu() override;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float fade() const;
  double now() const;

private:
  plm_t *video = nullptr;
  VK2DImage image = nullptr;
  VK2DTexture frame = nullptr;
  std::vector<uint8_t> rgba;
  int w = 0, h = 0;
  double length = 0, next = 0;
  float ending = -1;

  void decode();
  void finish();
};
