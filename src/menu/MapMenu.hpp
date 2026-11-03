#pragma once

#include "level/Chart.hpp"
#include "menu/Menu.hpp"

class MapMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }

private:
  double xPan = 0, yPan = 0;

  bool project(const ChartView &v, double wx, double wy, float &sx, float &sy) const;
  void campaign(Screen &s, const ChartView &v) const;
  float legend(Screen &s, float x, float y) const;
  int zoom = 1;
};
