#include "Crash.hpp"

#ifdef _WIN32
#include <windows.h>
#include <spdlog/spdlog.h>
#include <cstdint>

static LONG WINAPI onCrash(EXCEPTION_POINTERS *ep) {
  if (ep && ep->ExceptionRecord) {
    spdlog::critical("Crash 0x{:08X} at 0x{:X}",
                     ep->ExceptionRecord->ExceptionCode,
                     reinterpret_cast<uintptr_t>(ep->ExceptionRecord->ExceptionAddress));
    spdlog::default_logger()->flush();
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

void installCrashHandler() { SetUnhandledExceptionFilter(onCrash); }
#else
#include <execinfo.h>
#include <signal.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <cstring>

static void onCrash(int sig) {
  void *frames[48];
  int n = backtrace(frames, 48);
  char **names = backtrace_symbols(frames, n);
  spdlog::critical("Crash: signal {} ({})", sig, strsignal(sig));
  for (int i = 1; i < n; i++)
    spdlog::critical("  {}", names ? names[i] : "?");
  std::free(names);
  spdlog::default_logger()->flush();
  signal(sig, SIG_DFL);
  raise(sig);
}

void installCrashHandler() {
  for (int sig : {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS})
    signal(sig, onCrash);
}
#endif
