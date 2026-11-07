#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "campaign/Campaign.hpp"
#include "campaign/Quests.hpp"
#include "gui/Lang.hpp"
#include "hero/Hero.hpp"
#include "input/Input.hpp"
#include "item/loot/Item.hpp"
#include "item/loot/Loot.hpp"
#include "level/Chart.hpp"
#include "level/Structure.hpp"
#include "meta/Codex.hpp"
#include "meta/Heat.hpp"
#include "meta/Settings.hpp"

namespace {
struct Fuzz {
  uint64_t s = 0x2545F4914F6CDD1Dull;
  uint32_t next() {
    s ^= s << 13, s ^= s >> 7, s ^= s << 17;
    return uint32_t(s >> 16);
  }
  size_t below(size_t n) { return n ? next() % n : 0; }

  std::string mutate(std::string t) {
    static const char *const NUMBERS[] = {"-1", "0", "-2147483648", "2147483647", "99999999999", "nan", "inf",
                                          "-0.5", "1e30", "4294967295"};
    for (int k = 0, n = 1 + int(below(4)); k < n; k++) {
      switch (below(7)) {
      case 0:
        if (!t.empty())
          t[below(t.size())] = char(next());
        break;
      case 1:
        t.insert(below(t.size() + 1), 1, "0123456789- \n:_xq"[below(17)]);
        break;
      case 2:
        if (!t.empty()) {
          size_t at = below(t.size());
          t.erase(at, 1 + below(16));
        }
        break;
      case 3:
        t.resize(below(t.size() + 1));
        break;
      case 4: {
        size_t at = below(t.size() + 1), end = t.find('\n', at);
        if (end != std::string::npos)
          t.insert(at, t.substr(at, end - at + 1));
        break;
      }
      case 5: {
        size_t at = t.find_first_of("0123456789", below(t.size() + 1));
        if (at != std::string::npos) {
          size_t end = t.find_first_not_of("0123456789.", at);
          t.replace(at, (end == std::string::npos ? t.size() : end) - at, NUMBERS[below(10)]);
        }
        break;
      }
      default:
        t.insert(below(t.size() + 1), std::string(1 + below(40), char(next())));
      }
    }
    return t;
  }
};

void checkCampaign(const Campaign &c) {
  assert(c.hero >= 0 && c.hero < HERO_COUNT);
  assert(c.act >= 1 && c.act <= ACT_COUNT);
  assert(c.quest >= 0 && c.quest <= QUEST_COUNT);
  assert(c.lvl >= 1 && c.expNext > 0 && std::isfinite(c.exp) && std::isfinite(c.expNext));
  assert(c.gold >= 0 && c.points >= 0 && c.day >= 0 && c.rescued >= 0 && c.nightmare >= 0);
  for (int m : c.materials)
    assert(m >= 0);
  assert(std::isfinite(c.x) && std::isfinite(c.y));
  (void)HEROES[c.hero]->name;
  (void)quests::act(c).town;
  (void)quests::status(c);
  if (c.quest < QUEST_COUNT)
    (void)quests::rewardText(c.quest);
}
} // namespace

void testSaveFuzz() {
  constexpr int ROUNDS = 3000;
  auto start = std::chrono::steady_clock::now();
  Fuzz f;
  int accepted[4] = {};

  Settings set;
  set.language = "deutsch";
  std::string settingsText = set.save();
  for (int i = 0; i < ROUNDS; i++) {
    Settings s;
    s.parse(f.mutate(settingsText));
    for (int v : {s.master, s.music, s.effects, s.ambience, s.shake, s.flash})
      assert(v >= 0 && v <= 100);
    for (int v : {s.reduced, s.fullscreen, s.numbers, s.vsync, s.pixel, s.intro})
      assert(v == 0 || v == 1);
    assert(s.colours >= 0 && s.colours <= 4 && s.language.find('/') == std::string::npos &&
           s.language.find('.') == std::string::npos);
  }

  Input keys;
  std::string keysText = keys.saveKeys();
  for (int i = 0; i < ROUNDS; i++) {
    Input in;
    in.parseKeys(f.mutate(keysText));
    for (SDL_Scancode sc : in.keymap)
      assert(sc > SDL_SCANCODE_UNKNOWN && sc < SDL_SCANCODE_COUNT);
  }

  namespace fs = std::filesystem;
  fs::path dir = fs::temp_directory_path() / "mellangan-fuzz";
  fs::create_directories(dir);
  Codex codex;
  codex.runs = 12, codex.dawns = 3, codex.start = 5, codex.heat = 4, codex.bestTime = 812.5f, codex.feats = 0x1234;
  codex.save((dir / "codex.txt").string());
  std::stringstream codexFile;
  codexFile << std::ifstream(dir / "codex.txt").rdbuf();
  std::string codexText = codexFile.str();
  for (int i = 0; i < ROUNDS; i++) {
    Codex c;
    if (!c.parse(f.mutate(codexText)))
      continue;
    accepted[0]++;
    assert(c.start >= 0 && c.start < HERO_COUNT && c.heat >= 0 && c.heat <= HEAT_MAX);
    assert(std::isfinite(c.bestTime) && c.bestTime >= 0 && std::isfinite(c.bestEndless) && c.bestEndless >= 0);
    (void)HEROES[c.start]->name;
  }

  Rng kept = rng;
  Rng roll;
  roll.state = 77;
  Campaign camp;
  camp.world = "ABCDEFGH", camp.hero = 7, camp.act = 3, camp.quest = 9, camp.lvl = 14, camp.gold = 320;
  camp.towers = {0, 2}, camp.delves = {{3, 4}};
  camp.bag.push_back(rollItem(12, 0, false, roll, Rarity::Rare));
  camp.stash.push_back(rollItem(8, 0, true, roll, Rarity::Magic));
  camp.gear[0] = rollItem(10, 0, false, roll, Rarity::Unique), camp.worn[0] = true;
  std::string campaignText = camp.save();
  {
    Campaign back;
    assert(Campaign::load(campaignText, back) && back.save() == campaignText);
  }
  for (int i = 0; i < ROUNDS; i++) {
    Campaign c;
    if (!Campaign::load(f.mutate(campaignText), c))
      continue;
    accepted[1]++;
    checkCampaign(c);
  }
  rng = kept;

  Chart chart;
  for (int i = 0; i < 40; i++)
    chart.unveil(i * 90.0, -i * 70.0, 0);
  chart.save((dir / "chart.bin").string());
  std::stringstream chartFile;
  chartFile << std::ifstream(dir / "chart.bin", std::ios::binary).rdbuf();
  std::string chartBytes = chartFile.str();
  for (int i = 0; i < 300; i++) {
    std::string bytes = i % 4 == 0 ? f.mutate(chartBytes) : chartBytes;
    for (int k = 0, n = i % 4 == 0 ? 0 : 1 + int(f.below(64)); k < n; k++)
      bytes[i % 2 ? 64 + f.below(bytes.size() - 64) : f.below(bytes.size())] = char(f.next());
    std::ofstream(dir / "chart.bin", std::ios::binary | std::ios::trunc) << bytes;
    Chart c;
    c.load((dir / "chart.bin").string());
    fs::remove(dir / "again.bin");
    c.save((dir / "again.bin").string());
    std::stringstream again;
    again << std::ifstream(dir / "again.bin", std::ios::binary).rdbuf();
    std::string saved = again.str();
    if (saved.size() < 12)
      continue;
    accepted[2]++;
    uint32_t head[3];
    std::memcpy(head, saved.data(), sizeof head);
    assert(saved.size() == 12 + head[1] * (8 + Chart::PAGE * Chart::PAGE) + head[2] * sizeof(Structure));
    for (uint32_t k = 0; k < head[1]; k++)
      for (int cell = 0; cell < Chart::PAGE * Chart::PAGE; cell++)
        assert((uint8_t(saved[12 + k * (8 + Chart::PAGE * Chart::PAGE) + 8 + cell]) & ~Chart::WATER) <= BIOME_COUNT);
    for (uint32_t k = 0; k < head[2]; k++) {
      Structure site;
      std::memcpy(&site, saved.data() + 12 + head[1] * (8 + Chart::PAGE * Chart::PAGE) + k * sizeof(Structure),
                  sizeof site);
      assert(unsigned(site.kind) < unsigned(StructureKind::Count));
    }
  }

  std::string langText = langTemplate() + "~Arcane Staff\tBaton\n~Slay {} {}\tTuez {} {}\n";
  for (int i = 0; i < 300; i++) {
    Lang l;
    Glossary g;
    std::string t = f.mutate(langText);
    accepted[3] += l.parse(t) > 0;
    g.parse(t);
    for (int k = 0; k < LANG_COUNT; k++) {
      const char *s = l.*LANG_KEYS[k].text;
      if (std::strchr(s, '{'))
        (void)say(s, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    }
    for (auto &[english, word] : g.words)
      if (word.find('{') != std::string::npos)
        (void)say(word.c_str(), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  }

  fs::remove_all(dir);
  double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  std::cout << "[TEST] Save fuzz: " << ROUNDS << " mutations each of settings, keys, codex and campaign, 300 language files "
            << "and 300 charts; accepted codex " << accepted[0] << ", campaign " << accepted[1] << ", chart "
            << accepted[2] << ", language " << accepted[3] << ", every accepted value in range (" << int(ms)
            << " ms): PASS" << std::endl;
}
