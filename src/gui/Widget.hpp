#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <string>

#include "gfx/Screen.hpp"

class Input;

namespace gui {

class Widget {
public:
  SDL_FRect bounds;

  explicit Widget(SDL_FRect bounds) : bounds(bounds) {}
  virtual ~Widget() = default;
  virtual void render(Screen &s) const = 0;
  bool contains(float x, float y) const;
};

class Panel : public Widget {
public:
  Color background, border;
  float borderWidth;

  Panel(SDL_FRect bounds, Color background, Color border, float borderWidth);
  void render(Screen &s) const override;
};

class ChoiceCard : public Panel {
public:
  Color accent;
  float unit, opacity;
  bool selected;

  ChoiceCard(SDL_FRect bounds, Color accent, float unit, float opacity,
             bool selected);
  void render(Screen &s) const override;
};

class TextButton : public Panel {
public:
  std::string label;
  float textY, textSize;
  Color textColor;

  TextButton(SDL_FRect bounds, Color background, Color border, float borderWidth,
             std::string label, float textY, float textSize, Color textColor);
  void render(Screen &s) const override;
};

class KeyBadge : public TextButton {
public:
  KeyBadge(SDL_FRect bounds, std::string label, Color accent, float unit,
           float textSize, float opacity, bool selected);
};

struct ChoiceInput {
  int activated = -1;
  bool selectionChanged = false;
};

class ChoiceList {
public:
  int selected;
  explicit ChoiceList(int selected = 0) : selected(selected) {}
  static std::array<SDL_FRect, 4> layout(const Screen &s, int count);
  static float entrance(float time, int index);
  ChoiceInput update(const Input &input, const std::array<SDL_FRect, 4> &bounds,
                     int count, bool ready);
};
}
