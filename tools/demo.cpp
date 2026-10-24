#include <array>
#include <iostream>

#include "core/cpu_core.h"
#include "debugger/debugger.h"
#include "decoder/decoder.h"
#include "decoder/fetcher.h"
#include "executor/executor.h"
#include "memory/machine.h"

int main() {
  c166::Machine machine;
  c166::Fetcher fetcher;
  c166::Decoder decoder;
  c166::Executor executor;
  c166::CpuCore core(machine, fetcher, decoder, executor);
  c166::Debugger debugger(core);

  // MOV R0,#1; MOV R1,#2; ADD R0,R1; JMPR cc_UC,-1 (ISM pp. 41,91,96).
  constexpr std::array<c166::Byte, 12> kProgram = {0xE6, 0xF0, 0x01, 0x00, 0xE6, 0xF1,
                                                   0x02, 0x00, 0x00, 0x01, 0x0D, 0xFF};
  const auto loaded = core.LoadBinary(0, kProgram);
  if (!loaded.Ok()) {
    std::cerr << loaded.Error().message << '\n';
    return 1;
  }
  const auto started = core.SetEntryPoint({0, 0});
  if (!started.Ok()) {
    std::cerr << started.Error().message << '\n';
    return 1;
  }
  const auto run = debugger.Run({100, c166::CodeAddress{0, 10}});
  const auto snapshot = debugger.Snapshot();
  std::cout << "instructions=" << run.completed_instructions << " R0=" << snapshot.cpu.registers[0]
            << " R1=" << snapshot.cpu.registers[1] << " IP=" << snapshot.cpu.pc.offset << '\n';
  return run.reason == c166::StopReason::kStopAddress && snapshot.cpu.registers[0] == 3 ? 0 : 1;
}
