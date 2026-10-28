#pragma once

#include <atomic>
#include <memory>

#include "SpriteObject.hpp"

class Debris : public SpriteObject {
public:
  static constexpr int MAX = 1200;
  inline static std::atomic<int> alive{0};

  int life, icon = 0;
  double drag = 0.998, bounce = 0.6, gravity = 0.08;
  Color color = col::white;

  Debris(double x, double y, double z);
  ~Debris() override { alive--; }
  static bool room();
  static void gore(Level &level, double x, double y, double z, Color blood,
                   bool meat);

  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;

protected:
  virtual void collide(double xxa, double yya, double zza);
  void shed(std::unique_ptr<Debris> d, double keep, double carry);

private:
  void attemptMove();
  void move(double xxa, double yya, double zza);
};
