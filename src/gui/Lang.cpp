#include "gui/Lang.hpp"

#include <algorithm>
#include <cstring>
#include <deque>

Lang lang;

#define K(m) {#m, &Lang::m}
const LangKey LANG_KEYS[] = {
    K(world),

    K(paused), K(pauseResume), K(pauseCodex), K(pauseSettings), K(pauseQuit),

    K(settings), K(settingsHint), K(settingsPadHint), K(masterVolume), K(music), K(effects), K(ambience),
    K(screenShake), K(flashes), K(particles), K(damageNumbers), K(colours), K(fullscreen), K(scaling),
    K(vsync), K(controls), K(off), K(on), K(full), K(reduced), K(standard), K(deuteranopia), K(protanopia),
    K(tritanopia), K(monochrome), K(fill), K(pixelPerfect),

    K(gamepad), K(resetDefaults), K(pressButton), K(pressKey), K(newButton), K(padControlsHint), K(newKey),
    K(keyControlsHint),

    K(fallen), K(felledBy), K(overwhelmed), K(endlessRun), K(tip), K(riseAgain),

    K(dawnBreaks), K(conquered), K(anotherNight), K(keepGoing)};
#undef K
const int LANG_COUNT = int(sizeof LANG_KEYS / sizeof *LANG_KEYS);
static_assert(sizeof LANG_KEYS / sizeof *LANG_KEYS * sizeof(const char *) == sizeof(Lang));

int Lang::parse(std::string_view text) {
  static std::deque<std::string> held;
  static const Lang english;
  int taken = 0;
  for (size_t at = 0; at < text.size();) {
    size_t end = std::min(text.find('\n', at), text.size());
    std::string_view line = text.substr(at, end - at);
    at = end + 1;
    if (!line.empty() && line.back() == '\r')
      line.remove_suffix(1);
    size_t gap = line.find(' ');
    if (gap == std::string_view::npos || line[0] == '#')
      continue;
    std::string_view key = line.substr(0, gap), value = line.substr(gap + 1);
    for (int i = 0; i < LANG_COUNT; i++) {
      const LangKey &k = LANG_KEYS[i];
      std::string_view was = english.*k.text;
      if (key != k.key || std::count(was.begin(), was.end(), '{') != std::count(value.begin(), value.end(), '{'))
        continue;
      this->*k.text = held.emplace_back(value).c_str();
      taken++;
    }
  }
  return taken;
}
