#pragma once

#include "entity/Status.hpp"
#include "gfx/Screen.hpp"

inline Color statusColor(StatusKind kind) {
  constexpr Color colors[] = {{1, 0.42f, 0.12f, 1}, {0.45f, 0.85f, 1, 1},
                              {0.4f, 1, 0.3f, 1}, {1, 0.35f, 0.8f, 1}};
  return colors[int(kind)];
}

inline const char *statusName(StatusKind kind) {
  constexpr const char *names[] = {"BURN", "CHILL", "POISON", "MARKED"};
  return names[int(kind)];
}

inline void statusIcons(Screen &s, const Status &status, double x, double y) {
  int count = 0;
  for (int i = 0; i < int(StatusKind::Count); i++)
    count += status.power(StatusKind(i)) > 0;
  double px = x - count * 3.0;
  for (int i = 0; i < int(StatusKind::Count); i++) {
    StatusKind kind = StatusKind(i);
    if (status.power(kind) <= 0)
      continue;
    Color c = statusColor(kind);
    s.rect(px - 1, y - 1, 6, 6, {0.02f, 0.02f, 0.04f, 0.8f});
    if (kind == StatusKind::Burn) {
      s.rect(px + 1, y, 2, 4, c);
      s.rect(px, y + 2, 4, 2, c);
    } else if (kind == StatusKind::Chill) {
      s.rect(px + 1, y, 2, 4, c);
      s.rect(px, y + 1, 4, 2, c);
    } else if (kind == StatusKind::Poison) {
      s.rect(px + 1, y, 2, 1, c);
      s.rect(px, y + 1, 4, 2, c);
      s.rect(px + 1, y + 3, 2, 1, c);
    } else {
      s.rect(px, y, 1, 4, c);
      s.rect(px + 3, y, 1, 4, c);
      s.rect(px + 1, y + 1, 2, 2, c);
    }
    px += 6;
  }
}
