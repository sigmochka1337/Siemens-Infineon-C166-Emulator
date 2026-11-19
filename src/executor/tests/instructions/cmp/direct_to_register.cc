#include "instructions/instruction_test.h"

namespace c166::test::cmp {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMP direct memory");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kMDH, 0);
  machine.Write16(0xFA80, 1);

  ExecuteInstruction(checks, executor, Mnemonic::kCmp, 4, std::nullopt,
                     std::make_unique<RegisterOperand>(kMDH),
                     std::make_unique<DirectMemoryOperand>(0xFA80));

  checks.Equal("left operand", machine.ReadRegister(kMDH), 0);
  checks.Equal("memory operand", machine.Read16(0xFA80), 1);
  CheckCompleted(checks, machine, 4, kControlPsw | kCarry | kNegative);
}

}  // namespace c166::test::cmp
