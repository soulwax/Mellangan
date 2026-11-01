#pragma once

#include "item/weapon/form/FormBehavior.hpp"

class BoltForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &s) const override;
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void trail(RecipeShot &s) const override { wake(s); }
  bool flies() const override { return true; }
};

class VolleyForm : public BoltForm {
public:
  void fire(const FormCast &c) const override;
};

class SeekerForm : public BoltForm {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 3.0f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
};

class BoomerangForm : public BoltForm {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &s) const override;
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
};

class OrbitForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 3.2f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void trail(RecipeShot &s) const override { wake(s); }
  float knock() const override { return 60; }
};

class NovaForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 0.45f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void light(RecipeShot &s, Screen &screen) const override;
  bool area() const override { return true; }
};

class RainForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 0.6f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void light(RecipeShot &s, Screen &screen) const override;
  bool explodes() const override { return false; }
};

class FieldForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 3.0f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void light(RecipeShot &s, Screen &screen) const override;
  void trail(RecipeShot &s) const override;
  float knock() const override { return 0; }
  bool area() const override { return true; }
};

class CleaveForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 0.24f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void light(RecipeShot &s, Screen &screen) const override;
};

class LobForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 0.55f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void expire(RecipeShot &s) const override;
};

class TrapForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 10; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void light(RecipeShot &s, Screen &screen) const override;
};

class BeamForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 0.28f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void light(RecipeShot &s, Screen &screen) const override;
};

class WhipForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 0.3f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  float knock() const override { return 70; }
};

class WallForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 4.0f; }
  void tick(RecipeShot &s, Mob &p, float speed) const override;
  void render(RecipeShot &s, Screen &screen) const override;
  void light(RecipeShot &s, Screen &screen) const override;
  float knock() const override { return 20; }
};

class SummonForm : public FormBehavior {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &) const override { return 0; }
  void tick(RecipeShot &, Mob &, float) const override {}
  void render(RecipeShot &, Screen &) const override {}
  void light(RecipeShot &, Screen &) const override {}
  float knock() const override { return 40; }
};

class SpiralForm : public BoltForm {
public:
  void fire(const FormCast &c) const override;
  float life(const RecipeShot &s) const override;
  void tick(RecipeShot &s, Mob &p, float speed) const override;
};
