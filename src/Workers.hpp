#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class Workers {
public:
  typedef void (*Job)(void *ctx, int begin, int end);

  static Workers &shared();
  ~Workers();
  void run(int count, int chunk, Job job, void *ctx);
  int size() const { return int(threads.size()) + 1; }

private:
  Workers();
  void loop();
  void work();

  std::vector<std::thread> threads;
  std::mutex mutex;
  std::condition_variable wake, done;
  std::atomic<int> next{0};
  Job job = nullptr;
  void *ctx = nullptr;
  int count = 0, chunk = 1, busy = 0;
  unsigned generation = 0;
  bool quit = false;
};
