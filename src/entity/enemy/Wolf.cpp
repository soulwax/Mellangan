#include "entity/enemy/Wolf.hpp"

Color Wolf::tint() const {
  if (frenzy > 0)
    return {1.0f, 0.72f, 0.72f, 1.0f};
  return col::white;
}
