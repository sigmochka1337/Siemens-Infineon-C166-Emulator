#include "instructions/instruction_test.h"

namespace c166::test::nop {

void TestNoOperands(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("NOP preserves registers and wraps IP within the segment");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine, kInitialPsw, {3, 0xFFFE});
  machine.WriteRegister(kR7, 0x2468);
  ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
  CheckCompleted(checks, machine, {3, 0}, kInitialPsw);
  checks.Equal("R7", machine.ReadRegister(kR7), 0x2468);
}

}  // namespace c166::test::nop
