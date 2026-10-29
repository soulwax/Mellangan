#include "gui/Widget.hpp"

#include <algorithm>

#include "Math.hpp"
#include "input/Input.hpp"

namespace gui {
bool Widget::contains(float x, float y) const {
  SDL_FPoint p{x, y};
  return SDL_PointInRectFloat(&p, &bounds);
}

Panel::Panel(SDL_FRect bounds, Color background, Color border, float borderWidth)
    : Widget(bounds), background(background), border(border), borderWidth(borderWidth) {}

void Panel::render(Screen &s) const {
  s.rect(bounds.x, bounds.y, bounds.w, bounds.h, background);
  if (borderWidth > 0)
    s.frame(bounds.x, bounds.y, bounds.w, bounds.h, borderWidth, border);
}

ChoiceCard::ChoiceCard(SDL_FRect bounds, Color accent, float unit, float opacity,
                       bool selected)
    : Panel(bounds, col::panel.alpha(0.92f * opacity),
            accent.alpha((selected ? 1.0f : 0.65f) * opacity),
            selected ? 2 * unit : unit),
      accent(accent), unit(unit), opacity(opacity), selected(selected) {}

void ChoiceCard::render(Screen &s) const {
  s.rect(bounds.x, bounds.y, bounds.w, bounds.h, background);
  if (selected)
    s.rect(bounds.x, bounds.y, bounds.w, bounds.h, accent.alpha(0.12f * opacity));
  s.frame(bounds.x, bounds.y, bounds.w, bounds.h, borderWidth, border);
  if (!selected)
    return;
  float x = bounds.x, y = bounds.y, w = bounds.w, h = bounds.h, n = 7 * unit;
  Color c = col::white.alpha(opacity);
  s.line(x, y, x + n, y, unit, c);
  s.line(x, y, x, y + n, unit, c);
  s.line(x + w, y, x + w - n, y, unit, c);
  s.line(x + w, y, x + w, y + n, unit, c);
  s.line(x, y + h, x + n, y + h, unit, c);
  s.line(x, y + h, x, y + h - n, unit, c);
  s.line(x + w, y + h, x + w - n, y + h, unit, c);
  s.line(x + w, y + h, x + w, y + h - n, unit, c);
}

TextButton::TextButton(SDL_FRect bounds, Color background, Color border,
                       float borderWidth, std::string label, float textY,
                       float textSize, Color textColor)
    : Panel(bounds, background, border, borderWidth), label(std::move(label)),
      textY(textY), textSize(textSize), textColor(textColor) {}

void TextButton::render(Screen &s) const {
  Panel::render(s);
  s.textCenter(label, bounds.x + bounds.w * 0.5f, textY, textSize, textColor);
}

KeyBadge::KeyBadge(SDL_FRect bounds, std::string label, Color accent, float unit,
                   float textSize, float opacity, bool selected)
    : TextButton(bounds, {0.10f, 0.08f, 0.15f, 0.85f * opacity},
                 (selected ? accent : col::muted * 0.45f).alpha(opacity), unit,
                 std::move(label), bounds.y + (bounds.h - 16 * textSize) * 0.5f,
                 textSize, (selected ? col::white : col::muted).alpha(opacity)) {}

std::array<SDL_FRect, 4> ChoiceList::layout(const Screen &s, int count) {
  std::array<SDL_FRect, 4> result{};
  count = std::clamp(count, 0, int(result.size()));
  if (!count)
    return result;
  float u = s.u, w = std::min(s.w * 0.90f, 440 * u), h = 46 * u, gap = 6 * u;
  float y = s.h * 0.5f - (count * h + (count - 1) * gap) * 0.5f + 10 * u;
  for (int i = 0; i < int(result.size()); i++)
    result[i] = {(s.w - w) * 0.5f, y + i * (h + gap), w, h};
  return result;
}

float ChoiceList::entrance(float time, int index) {
  float in = clampf((time - index * 0.05f) / 0.22f, 0, 1);
  return 1 - (1 - in) * (1 - in) * (1 - in);
}

ChoiceInput ChoiceList::update(const Input &input,
                               const std::array<SDL_FRect, 4> &bounds,
                               int count, bool ready) {
  ChoiceInput result;
  count = std::clamp(count, 0, int(bounds.size()));
  if (!count)
    return result;
  selected = std::clamp(selected, 0, count - 1);
  int previous = selected;
  if (input.up.clicked)
    selected = (selected + count - 1) % count;
  if (input.down.clicked)
    selected = (selected + 1) % count;
  const Key *keys[] = {&input.one, &input.two, &input.three, &input.four};
  SDL_FPoint p{input.mouseX, input.mouseY};
  for (int i = 0; i < count; i++) {
    if (keys[i]->clicked) {
      result.activated = i;
      return result;
    }
    if (!SDL_PointInRectFloat(&p, &bounds[i]))
      continue;
    if (input.mouseMoved)
      selected = i;
    if (input.mouseClicked && ready) {
      result.activated = i;
      return result;
    }
  }
  result.selectionChanged = selected != previous;
  if (input.confirm.clicked && ready)
    result.activated = selected;
  return result;
}
}
