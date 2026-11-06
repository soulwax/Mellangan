#include "Workers.hpp"

#include <algorithm>

Workers &Workers::shared() {
  static Workers workers;
  return workers;
}

Workers::Workers() {
  unsigned n = std::max(2u, std::thread::hardware_concurrency()) - 1;
  for (unsigned i = 0; i < n; i++)
    threads.emplace_back([this] { loop(); });
}

Workers::~Workers() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    quit = true;
  }
  wake.notify_all();
  for (std::thread &t : threads)
    t.join();
}

void Workers::run(int n, int size, Job j, void *c) {
  if (n <= 0)
    return;
  if (threads.empty() || n <= size)
    return j(c, 0, n);
  uint32_t gen;
  {
    std::lock_guard<std::mutex> lock(mutex);
    job = j, ctx = c, count = n, chunk = size, finished = 0, total = (n + size - 1) / size;
    gen = ++generation;
    next.store(uint64_t(gen) << 32);
  }
  wake.notify_all();
  int mine = work(gen, j, c, n, size);
  std::unique_lock<std::mutex> lock(mutex);
  finished += mine;
  done.wait(lock, [this] { return finished == total; });
}

int Workers::work(uint32_t gen, Job j, void *c, int n, int size) {
  int chunks = 0;
  for (uint64_t v = next.load(); uint32_t(v >> 32) == gen && int(uint32_t(v)) < n;) {
    if (!next.compare_exchange_weak(v, v + uint64_t(size)))
      continue;
    int begin = int(uint32_t(v));
    j(c, begin, std::min(begin + size, n));
    chunks++;
    v = next.load();
  }
  return chunks;
}

void Workers::loop() {
  uint32_t seen = 0;
  for (;;) {
    Job j;
    void *c;
    int n, size;
    {
      std::unique_lock<std::mutex> lock(mutex);
      wake.wait(lock, [&] { return quit || generation != seen; });
      if (quit)
        return;
      seen = generation, j = job, c = ctx, n = count, size = chunk;
    }
    if (int chunks = work(seen, j, c, n, size)) {
      std::lock_guard<std::mutex> lock(mutex);
      if ((finished += chunks) == total)
        done.notify_one();
    }
  }
}
