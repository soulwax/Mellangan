#pragma once

#include <cstdint>
#include <map>
#include <memory>

class Level;
class Screen;

enum class Stuff : uint8_t { None, Wet, Burning, Frozen, Oiled, Spored, Charged, Ash, Limed, Count };
enum class Cause : uint8_t { Fire, Chill, Water, Charge, Spore, Count };

class Matter {
public:
  static constexpr float STEP = 0.1f;
  static constexpr int SPREAD = 10, PAGE = 16;

  bool (*fuel)(int x, int y) = nullptr;

  Stuff at(int x, int y) const;
  float left(int x, int y) const;
  void set(int x, int y, Stuff s, float seconds);
  void tick(Level &level);
  void render(Screen &s, float time) const;
  bool empty() const { return pages.empty(); }
  size_t pageCount() const { return pages.size(); }
  void clear() { pages.clear(); }

private:
  struct Page {
    uint8_t kind[PAGE * PAGE]{};
    uint16_t time[PAGE * PAGE]{};
    int live = 0;
  };
  std::map<uint64_t, std::unique_ptr<Page>> pages;
  float clock = 0;
  int steps = 0;

  static uint64_t keyOf(int px, int py) { return uint64_t(uint32_t(px)) << 32 | uint32_t(py); }
  const Page *find(int x, int y) const;
};
