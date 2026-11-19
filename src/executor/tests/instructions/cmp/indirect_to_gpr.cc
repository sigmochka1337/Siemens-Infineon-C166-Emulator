#include "instructions/instruction_test.h"

namespace c166::test::cmp {

void TestIndirectToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMP indirect");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR4, 0);
  machine.WriteRegister(kR3, 0xFA80);
  machine.Write16(0xFA80, 1);

  ExecuteInstruction(checks, executor, Mnemonic::kCmp, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR4),
                     IndirectMemoryOperand::CreatePlain(kR3));

  checks.Equal("left operand", machine.ReadRegister(kR4), 0);
  checks.Equal("pointer", machine.ReadRegister(kR3), 0xFA80);
  CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);
}

}  // namespace c166::test::cmp
