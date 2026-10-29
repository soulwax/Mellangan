#pragma once

#include "Math.hpp"
#include "entity/Pool.h"
#include "gfx/Screen.hpp"

class Level;

class SpriteObject {
public:
  double x = 0, y = 0, z = 0;
  double xa = 0, ya = 0, za = 0;
  int xp = 0, yp = 0;
  Sprite sprite{};
  bool removed = false;
  Level *level = nullptr;

  virtual ~SpriteObject() = default;
  static void *operator new(size_t size) { return pool_alloc(size); }
  static void operator delete(void *p, size_t size) { pool_free(p, size); }

  virtual void tick() {}

  void draw(Screen &s) {
    place(z);
    render(s);
  }
  void drawShadow(Screen &s) {
    place(0);
    renderShadow(s);
  }
  void drawLight(Screen &s) {
    place(z);
    renderLight(s);
  }

  virtual void render(Screen &s) {
    if (sprite.w > 0)
      s.sprite(sprite, xp, yp, sprite.w * 0.5f, sprite.h);
  }

  virtual void renderShadow(Screen &) {}
  virtual void renderLight(Screen &) {}

  virtual float occlusion(float &width) const { return width = 0, 0.0f; }
  virtual int layer() const { return 1; }
  virtual bool cosmeticParticle() const { return false; }
  void remove() { removed = true; }

private:
  void place(double height) {
    xp = int(std::floor(x)), yp = int(std::floor(y - height));
  }
};
