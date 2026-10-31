#pragma once

#include "item/Upgrade.hpp"

class EchoingBlink : public Upgrade {
public:
  EchoingBlink();
  void cast(Mob &p) override;
};
