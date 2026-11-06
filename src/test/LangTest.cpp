#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <cstring>
#include <iostream>
#include <string>

#include "gui/Lang.hpp"

void testLang() {
  for (int i = 0; i < LANG_COUNT; i++) {
    const LangKey &a = LANG_KEYS[i];
    const char *text = lang.*a.text;
    assert(text && *text);
    if (std::strchr(text, '{'))
      (void)say(text, 0, 0, 0, 0);
    for (int j = i + 1; j < LANG_COUNT; j++)
      assert(std::strcmp(a.key, LANG_KEYS[j].key) != 0 && a.text != LANG_KEYS[j].text);
  }

  Lang l;
  assert(l.parse("paused PAUSA\r\n# note\nfelledBy abattu par {}\nfelledBy abattu\nnope x\nworld") == 2);
  assert(std::strcmp(l.paused, "PAUSA") == 0 && say(l.felledBy, "wolf") == "abattu par wolf");
  assert(std::strcmp(l.world, lang.world) == 0 && std::strcmp(lang.paused, "PAUSED") == 0);
  std::cout << "[TEST] Lang: " << LANG_COUNT << " strings, unique keys, formats parse, overrides keep placeholders"
            << std::endl;
}
