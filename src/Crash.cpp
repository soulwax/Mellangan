#include "Crash.hpp"

#ifdef _WIN32
#include <windows.h>

#include <dbghelp.h>
#include <spdlog/spdlog.h>


static LONG WINAPI onCrash(EXCEPTION_POINTERS *ep) {
  HANDLE proc = GetCurrentProcess();
  SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
  SymInitialize(proc, nullptr, TRUE);

  CONTEXT ctx = *ep->ContextRecord;
  STACKFRAME64 frame{};
  frame.AddrPC = {ctx.Rip, 0, AddrModeFlat};
  frame.AddrFrame = {ctx.Rbp, 0, AddrModeFlat};
  frame.AddrStack = {ctx.Rsp, 0, AddrModeFlat};

  spdlog::critical("Crash 0x{:08X} at 0x{:X}",
                   ep->ExceptionRecord->ExceptionCode, ctx.Rip);
  for (int i = 0;
       i < 40 && StackWalk64(IMAGE_FILE_MACHINE_AMD64, proc, GetCurrentThread(),
                             &frame, &ctx, nullptr, SymFunctionTableAccess64,
                             SymGetModuleBase64, nullptr);
       i++) {
    DWORD64 addr = frame.AddrPC.Offset, disp = 0;
    alignas(SYMBOL_INFO) char buf[sizeof(SYMBOL_INFO) + 256]{};
    auto *sym = reinterpret_cast<SYMBOL_INFO *>(buf);
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);
    sym->MaxNameLen = 255;
    const char *name = SymFromAddr(proc, addr, &disp, sym) ? sym->Name : "?";
    IMAGEHLP_LINE64 line{};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    DWORD lineDisp = 0;
    if (SymGetLineFromAddr64(proc, addr, &lineDisp, &line))
      spdlog::critical("  {} ({}:{})", name, line.FileName, line.LineNumber);
    else
      spdlog::critical("  {} [0x{:X}]", name, addr);
  }
  spdlog::default_logger()->flush();
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
