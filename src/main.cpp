#include <iostream>

#include "debugger/debugger.h"
#include "debugger/debugger_cli.h"
#include "executor/executor.h"
#include "fetcher.h"
#include "memory/machine.h"
#include "memory/raw_memory.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Использование: \n> c166 <файл intel hex>\n";
    return 1;
  }
  c166::RawMemory raw_memory;
  c166::Machine machine(raw_memory);
  c166::Fetcher fetcher(machine);
  c166::Executor executor(machine);
  c166::Debugger debugger(machine, fetcher, executor);
  c166::DebuggerCli cli(debugger, std::cin, std::cout);
  machine.LoadIntelHexFile(argv[1]);

  cli.Loop();
}
