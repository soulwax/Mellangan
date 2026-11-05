#include "Crash.hpp"

#include <spdlog/spdlog.h>

#include <csignal>
#include <cstdio>
#include <cstring>

#include "Version.hpp"

namespace {
char context[160] = "title";
char dumpPath[1024] = "";
} // namespace

void crashContext(const std::string &text) {
  std::snprintf(context, sizeof context, "%s", text.c_str());
}

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>

namespace {
std::string where(const void *addr) {
  HMODULE m = nullptr;
  char name[MAX_PATH] = "?";
  if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                             GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         static_cast<LPCSTR>(addr), &m))
    GetModuleFileNameA(m, name, MAX_PATH);
  const char *base = std::strrchr(name, '\\');
  return fmt::format("{}+0x{:X}", base ? base + 1 : name,
                     uintptr_t(addr) - uintptr_t(m));
}

void report(const char *what, EXCEPTION_POINTERS *e) {
  spdlog::critical("Crash: {} in Mellangan v{} ({})", what, MELLANGAN_VERSION, context);
  if (e)
    spdlog::critical("  at {}", where(e->ExceptionRecord->ExceptionAddress));
  void *frames[48];
  USHORT n = CaptureStackBackTrace(1, 48, frames, nullptr);
  for (USHORT i = 0; i < n; i++)
    spdlog::critical("  {}", where(frames[i]));
  HANDLE f = *dumpPath ? CreateFileA(dumpPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL, nullptr)
                       : INVALID_HANDLE_VALUE;
  if (f != INVALID_HANDLE_VALUE) {
    MINIDUMP_EXCEPTION_INFORMATION info{GetCurrentThreadId(), e, FALSE};
    if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), f, MiniDumpNormal,
                          e ? &info : nullptr, nullptr, nullptr))
      spdlog::critical("Crash dump: {}", dumpPath);
    CloseHandle(f);
  }
  spdlog::default_logger()->flush();
}

LONG WINAPI onException(EXCEPTION_POINTERS *e) {
  char what[32];
  std::snprintf(what, sizeof what, "exception 0x%08lX", e->ExceptionRecord->ExceptionCode);
  report(what, e);
  return EXCEPTION_CONTINUE_SEARCH;
}

void onAbort(int) {
  report("abort", nullptr);
  std::signal(SIGABRT, SIG_DFL);
  std::raise(SIGABRT);
}
} // namespace

void installCrashHandler(const std::string &dir) {
  std::snprintf(dumpPath, sizeof dumpPath, "%scrash.dmp", dir.c_str());
  SetUnhandledExceptionFilter(onException);
  std::signal(SIGABRT, onAbort);
}
#else
#include <execinfo.h>

#include <cstdlib>

namespace {
void onCrash(int sig) {
  void *frames[48];
  int n = backtrace(frames, 48);
  char **names = backtrace_symbols(frames, n);
  spdlog::critical("Crash: signal {} ({}) in Mellangan v{} ({})", sig, strsignal(sig),
                   MELLANGAN_VERSION, context);
  for (int i = 1; i < n; i++)
    spdlog::critical("  {}", names ? names[i] : "?");
  std::free(names);
  spdlog::default_logger()->flush();
  signal(sig, SIG_DFL);
  raise(sig);
}
} // namespace

void installCrashHandler(const std::string &dir) {
  (void)dir, (void)dumpPath;
  for (int sig : {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS})
    signal(sig, onCrash);
}
#endif
