#undef NDEBUG
#include <cassert>

#include "test/Tests.hpp"

#include <iostream>

#include "meta/Content.hpp"

void testContentLinks() {
  std::vector<std::string> faults = contentFaults();
  for (const std::string &f : faults)
    std::cout << "[CONTENT] " << f << std::endl;
  assert(faults.empty());
  std::cout << "[TEST] Content tables name only real heroes, upgrades and recipes: PASS" << std::endl;
}
