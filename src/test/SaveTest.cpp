#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <SDL3/SDL.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "campaign/Campaign.hpp"
#include "level/Chart.hpp"
#include "level/World.hpp"
#include "meta/Codex.hpp"
#include "meta/Saves.hpp"
#include "meta/Settings.hpp"
#include "particle/TextParticle.hpp"
#include "particle/VisualDetail.hpp"

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
  c.runs = 2, c.slain[3] = 7, c.hints = 5, c.feats = 0x8000000000000005ull, c.bestEndless = 61.5f;
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
  assert(fresh.slain[3] == 8 && fresh.hints == 5 && fresh.feats == 0x8000000000000005ull &&
         fresh.bestEndless == 61.5f);
  assert(whole.find("\nversion 2\n") != std::string::npos);
  scribble(cf, whole.substr(0, whole.find("version")));
  Codex legacy;
  legacy.load(cf);
  assert(legacy.slain[3] == 8 && legacy.runs == 2);

  std::string pf = (dir / "campaign.txt").string();
  Campaign camp;
  camp.world = "SAVETEST", camp.gold = 5;
  assert(Saves::store(pf, camp.save()));
  camp.gold = 9;
  assert(Saves::store(pf, camp.save()));
  std::string text = Saves::slurp(pf);
  scribble(pf, "mellangan-campaign 99\n" + text.substr(text.find('\n') + 1));
  Campaign restored;
  assert(Campaign::load(Saves::recall(pf, Campaign::valid), restored) && restored.gold == 5);

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

  int lx = 0, ly = 0;
  for (int r = 0; r < 400 && !lx; r++)
    for (int cx = -r; cx <= r && !lx; cx++)
      for (int cy : {-r, r}) {
        bool all = true;
        for (int t = 0; t < Chart::CELL * Chart::CELL && all; t++)
          all = world::isWater(cx * Chart::CELL + t % Chart::CELL, cy * Chart::CELL + t / Chart::CELL);
        if (all) {
          lx = cx, ly = cy;
          break;
        }
      }
  assert(lx || ly);
  Chart wet;
  wet.unveil((lx * Chart::CELL + 2) * 16.0, (ly * Chart::CELL + 2) * 16.0, 0);
  assert(wet.at(lx, ly) & Chart::WATER);
  wet.save(mf);
  Chart dried;
  dried.load(mf);
  assert((dried.at(lx, ly) & Chart::WATER) && (dried.at(lx, ly) & Chart::BIOME) == (wet.at(lx, ly) & Chart::BIOME));
  Saves::erase(mf);

  Settings set;
  set.parse("master 40\nflash 300\nbogus 5\nfullscreen 1\nshake -20\n");
  assert(set.master == 40 && set.flash == 100 && set.fullscreen == 1 && set.shake == 0 &&
         set.music == 100 && set.reduced == 0);
  Settings again;
  again.parse(set.save());
  assert(again.save() == set.save() && again.numbers == 1 && again.vsync == 1);
  Settings crisp;
  crisp.parse("pixel_perfect 1\n");
  assert(crisp.pixel == 1 && set.pixel == 0);
  {
    TextParticle number(0, 0, 12, col::red);
    assert(showParticle(number, false, true) && showParticle(number, true, true) &&
           !showParticle(number, false, false));
  }

  fs::remove_all(dir);
  std::cout << "[TEST] Saves: writes go through a temp file and keep the last good copy, an empty, "
               "missing or damaged codex, campaign or chart falls back to it, the codex is versioned, and erase removes both: PASS"
            << std::endl;
}
