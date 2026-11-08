#include <array>
#include <iostream>

#include "debugger/debugger.h"
#include "debugger/debugger_cli.h"
#include "executor/executor.h"
#include "fetcher.h"
#include "memory/machine.h"
#include "memory/raw_memory.h"

int main() {
  c166::RawMemory raw_memory;
  c166::Machine machine(raw_memory);
  c166::Fetcher fetcher(machine);
  c166::Executor executor(machine);
  c166::Debugger debugger(machine, fetcher, executor);
  c166::DebuggerCli cli(debugger, std::cin, std::cout);

  constexpr std::array<c166::Byte, 4> kResetVector{
      0xFA, 0x00, 0x04, 0x02,  // jmps  0x00:main
  };

  // sum:
  //   ADD   R0, R1
  //   RET
  //
  // main:
  //   MOV   R0, #1
  //   MOV   R1, #2
  //   CALLA cc_UC, sum
  //   MOV   R0, #0
  //
  // end:
  //   JMPR cc_UC, end
  constexpr std::array<c166::Byte, 16> kCode{
      0x00, 0x01,              // sum:  add   R0, R1
      0xCB, 0x00,              //       ret
      0xE0, 0x10,              // main: mov   R0, #1
      0xE0, 0x21,              //       mov   R1, #2
      0xCA, 0x00, 0x00, 0x02,  //       calla cc_UC, sum
      0xE0, 0x00,              //       mov   R0, #0
      0x0D, 0xFF,              // end:  jmpr  cc_UC, end
  };
  
  machine.LoadBinary(0x0, kResetVector);
  machine.LoadBinary(0x000200, kCode);

  cli.Loop();
}
