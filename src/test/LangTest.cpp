#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

#include "gui/Lang.hpp"
#include "item/loot/Item.hpp"

void testLang() {
  for (int i = 0; i < LANG_COUNT; i++) {
    const LangKey &a = LANG_KEYS[i];
    const char *text = lang.*a.text;
    assert(text && *text && std::count(text, text + std::strlen(text), '{') <= 8);
    if (std::strchr(text, '{'))
      (void)say(text, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    for (int j = i + 1; j < LANG_COUNT; j++)
      assert(std::strcmp(a.key, LANG_KEYS[j].key) != 0 && a.text != LANG_KEYS[j].text);
  }

  Lang l;
  assert(l.parse("paused PAUSA\r\n# note\nfelledBy abattu par {}\nfelledBy abattu\nnope x\nworld\n"
                 "kilometres {:d} km\nkilometres {:.1f} km") == 3);
  assert(say(l.kilometres, 2.5f) == "2.5 km");
  assert(std::strcmp(l.paused, "PAUSA") == 0 && say(l.felledBy, "wolf") == "abattu par wolf");
  assert(std::strcmp(l.world, lang.world) == 0 && std::strcmp(lang.paused, "PAUSED") == 0);

  const char *staff = "Arcane Staff";
  assert(tr(staff) == staff && glossary.words.empty());
  Glossary g;
  assert(g.parse("~Arcane Staff\tBaton arcanique\r\n~Slay {} {}\tTuez {} {}\n~Slay {}\tTuez\n"
                 "~Bad {}\t{:d} x\nArcane Staff\tno tilde\n~\tempty\n") == 2);
  assert(std::strcmp(g(staff), "Baton arcanique") == 0 && g("Frost Nova") == std::string("Frost Nova") &&
         g(nullptr) == nullptr && say(g("Slay {} {}"), 3, "bats") == "Tuez 3 bats");
  Item it;
  it.rarity = Rarity::Superior;
  std::string english = it.name(), base = it.def().name;
  const char *superior = lang.itemSuperior;
  lang.itemSuperior = "{} superieur";
  glossary.words["Worn"] = "Use";
  glossary.words[base] = "Chose";
  assert(english == "Superior Worn " + base && it.name(true) == "Use Chose superieur" && it.name() == english);
  glossary.words.clear();
  lang.itemSuperior = superior;
  assert(it.name(true) == english);
  std::cout << "[TEST] Lang: " << LANG_COUNT << " strings, unique keys, formats parse, overrides keep placeholders; "
               "the glossary passes English through and swaps only well-formed translations"
            << std::endl;
}
