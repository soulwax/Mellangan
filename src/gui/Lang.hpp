#pragma once

#include <spdlog/fmt/fmt.h>

#include <string>
#include <string_view>
#include <utility>

struct Lang {
  const char *world = "world {}";

  const char *paused = "PAUSED", *pauseResume = "Enter / Esc to resume", *pauseCodex = "C codex",
             *pauseSettings = "O settings", *pauseQuit = "Q to quit to title";

  const char *settings = "SETTINGS", *settingsHint = "Up/Down choose   Left/Right change   Esc back",
             *settingsPadHint = "D-pad choose and change   B back", *masterVolume = "MASTER VOLUME",
             *music = "MUSIC", *effects = "EFFECTS", *ambience = "AMBIENCE", *screenShake = "SCREEN SHAKE",
             *flashes = "FLASHES", *particles = "PARTICLES", *damageNumbers = "DAMAGE NUMBERS",
             *colours = "COLOURS", *fullscreen = "FULLSCREEN", *scaling = "SCALING", *vsync = "VSYNC",
             *controls = "CONTROLS", *off = "OFF", *on = "ON", *full = "FULL", *reduced = "REDUCED",
             *standard = "STANDARD", *deuteranopia = "DEUTERANOPIA",
             *protanopia = "PROTANOPIA", *tritanopia = "TRITANOPIA", *monochrome = "MONOCHROME", *fill = "FILL",
             *pixelPerfect = "PIXEL PERFECT";

  const char *gamepad = "GAMEPAD", *resetDefaults = "RESET TO DEFAULTS", *pressButton = "PRESS A BUTTON",
             *pressKey = "PRESS A KEY", *newButton = "press the new button   B cancel",
             *padControlsHint = "D-pad choose   A rebind   B back   A, B, Start and the D-pad stay fixed",
             *newKey = "press the new key   Esc cancel",
             *keyControlsHint = "Up/Down choose   Enter rebind   Esc back   arrows, Enter, Esc, 1-4, Q stay fixed";

  const char *fallen = "YOU HAVE FALLEN", *felledBy = "felled by {}", *overwhelmed = "overwhelmed by the dark",
             *endlessRun = "{} into the endless night   best {}", *tip = "TIP  {}",
             *riseAgain = "R / Enter to rise again";

  const char *dawnBreaks = "DAWN BREAKS", *conquered = "The night was conquered at {}",
             *anotherNight = "R / Enter for another night",
             *keepGoing = "K  keep going into the endless night";

  int parse(std::string_view text);
};

extern Lang lang;

struct LangKey {
  const char *key;
  const char *Lang::*text;
};

extern const LangKey LANG_KEYS[];
extern const int LANG_COUNT;

template <class... A> std::string say(const char *f, A &&...a) {
  return fmt::format(fmt::runtime(f), std::forward<A>(a)...);
}
