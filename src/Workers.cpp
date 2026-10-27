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
  {
    std::lock_guard<std::mutex> lock(mutex);
    job = j, ctx = c, count = n, chunk = size, busy = int(threads.size()),
    generation++;
    next.store(0);
  }
  wake.notify_all();
  work();
  std::unique_lock<std::mutex> lock(mutex);
  done.wait(lock, [this] { return busy == 0; });
}

void Workers::work() {
  for (int begin; (begin = next.fetch_add(chunk)) < count;)
    job(ctx, begin, std::min(begin + chunk, count));
}

void Workers::loop() {
  unsigned seen = 0;
  for (;;) {
    {
      std::unique_lock<std::mutex> lock(mutex);
      wake.wait(lock, [&] { return quit || generation != seen; });
      if (quit)
        return;
      seen = generation;
    }
    work();
    std::lock_guard<std::mutex> lock(mutex);
    if (--busy == 0)
      done.notify_one();
  }
}
