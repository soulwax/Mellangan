#pragma once

#include <string>

class Level;

struct Bounty {
  int kind, arg, goal;
  bool done;

  std::string text() const;
  int progress(const Level &l) const;
  static void roll(Bounty *out, int n);
};
