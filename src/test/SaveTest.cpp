#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <SDL3/SDL.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "level/Chart.hpp"
#include "meta/Codex.hpp"
#include "meta/Saves.hpp"

namespace {
void scribble(const std::string &file, std::string_view text) {
  assert(SDL_SaveFile(file.c_str(), text.data(), text.size()));
}
} // namespace

void testSaves() {
  namespace fs = std::filesystem;
  fs::path dir = fs::temp_directory_path() / ("mellangan-saves-" + std::to_string(SDL_GetTicksNS()));
  fs::create_directories(dir);

  std::string f = (dir / "notes.txt").string();
  assert(Saves::store(f, "one\n") && Saves::store(f, "two\n"));
  assert(Saves::slurp(f) == "two\n" && Saves::slurp(f + ".bak") == "one\n" &&
         !fs::exists(f + ".tmp"));
  scribble(f, "");
  assert(Saves::recall(f) == "one\n");
  fs::remove(f);
  assert(Saves::recall(f) == "one\n");
  assert(Saves::store(f, "three\n") && Saves::slurp(f) == "three\n");
  Saves::erase(f);
  assert(!fs::exists(f) && !fs::exists(f + ".bak") && Saves::recall(f).empty());

  std::string cf = (dir / "codex.txt").string();
  Codex c;
  c.runs = 2, c.slain[3] = 7;
  c.save(cf);
  c.slain[3] = 8;
  c.save(cf);
  std::string whole = Saves::slurp(cf);
  scribble(cf, whole.substr(0, whole.size() / 2));
  Codex back;
  back.load(cf);
  assert(back.runs == 2 && back.slain[3] == 7);
  scribble(cf, "garbage"), scribble(cf + ".bak", "x");
  Codex none;
  none.load(cf);
  assert(none.runs == 0 && none.slain[3] == 0);
  scribble(cf, whole);
  Codex fresh;
  fresh.load(cf);
  assert(fresh.slain[3] == 8);

  std::string mf = (dir / "chart.bin").string();
  Chart chart;
  chart.unveil(8, 8, 12);
  int cell = chart.at(0, 0) & Chart::BIOME;
  assert(cell > 0);
  chart.save(mf);
  chart.save(mf);
  std::string bin = Saves::slurp(mf);
  scribble(mf, bin.substr(0, bin.size() - 5));
  Chart healed;
  healed.load(mf);
  assert(healed.at(0, 0) == cell);
  Saves::erase(mf);
  Chart lost;
  lost.load(mf);
  assert(lost.at(0, 0) == 0);

  fs::remove_all(dir);
  std::cout << "[TEST] Saves: writes go through a temp file and keep the last good copy, an empty, "
               "missing or damaged codex or chart falls back to it, and erase removes both: PASS"
            << std::endl;
}
