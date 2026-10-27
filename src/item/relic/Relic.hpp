#pragma once

#include "item/Upgrade.hpp"

class Relic : public Upgrade {
public:
  Relic(const char *name, Sprite icon, const char *desc, void (*grant)(Player &))
      : Upgrade(name, icon, {desc, desc, desc, desc, desc}, false),
        grant(grant) {
    rare = true;
  }
  void levelUp(Player &p) override {
    Upgrade::levelUp(p);
    grant(p);
  }

private:
  void (*grant)(Player &);
};
