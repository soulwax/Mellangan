#pragma once

#include <string>

#include "entity/pickup/Pickup.hpp"
#include "item/loot/Loot.hpp"

class Level;

class ItemDrop : public Pickup {
public:
  static constexpr float FLY = 0.35f;

  Drop drop;
  double sx, sy, tx, ty;

  ItemDrop(double x, double y, Drop drop, double tx, double ty);
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

  bool beams() const;
  bool labelled(bool held) const;
  std::string label() const;
  Color color() const;

  static void spill(Level &level, double x, double y, Source source, int mlvl,
                    Lean lean = Lean::None);

protected:
  float reach() const override;
  void take(Player &p) override;
};
