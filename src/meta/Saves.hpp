#pragma once

#include <string>
#include <string_view>
#include <vector>

enum class WorldMode { Classic, Random, Daily, Custom };

struct PastWorld {
  std::string code;
  float best;
  int runs;
  bool dawn;
};

class Saves {
public:
  static constexpr int HISTORY = 6;
  std::vector<PastWorld> history;

  void open();
  const std::string &dir() const { return path; }
  std::string read(const std::string &name) const;
  void write(const std::string &name, std::string_view data) const;
  void loadWorld(WorldMode &mode, std::string &code);
  void saveWorld(WorldMode mode, const std::string &code) const;
  void record(const std::string &code, float time, bool dawn);
  static std::string slurp(const std::string &file);

private:
  std::string path;
};
