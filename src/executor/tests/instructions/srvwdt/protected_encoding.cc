#include "instructions/instruction_test.h"

namespace c166::test::srvwdt {

void TestProtectedEncoding(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("SRVWDT loads WDTREL and clears the low byte");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  raw.Write8(0xFEAE, 0xDA);
  machine.Write16(0xFFAE, 0x5A81);
  ExecuteInstruction(checks, executor, Mnemonic::kSrvwdt, 4);
  checks.Equal("watchdog timer", machine.Read16(0xFEAE), 0x5A00);
  CheckCompleted(checks, machine, 4, kInitialPsw);

  checks.BeginCase("Broken SRVWDT protection saves the faulting IP");
  machine.Write16(0xFFAE, 0x3381);
  Instruction malformed{Mnemonic::kSrvwdt, std::nullopt, {}, 4};
  malformed.protection_fault = true;
  executor.Execute(malformed);
  CheckHardwareTrap(checks, machine, kClassBVector, 0x0008);
  checks.Equal("saved IP", ReadWord(raw, kResetStackPointer - 6), kStart.offset + 4);
  checks.Equal("watchdog timer", machine.Read16(0xFEAE), 0x5A00);
}

}  // namespace c166::test::srvwdt
