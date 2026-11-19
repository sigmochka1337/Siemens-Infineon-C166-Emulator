#include "instructions/instruction_test.h"

namespace c166::test::cmp {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMP register pair");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR4, 0);
  machine.WriteRegister(kR6, 1);
  ExecuteInstruction(checks, executor, Mnemonic::kCmp, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR4),
                     std::make_unique<RegisterOperand>(kR6));

  checks.Equal("left operand", machine.ReadRegister(kR4), 0);

  CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);
}

}  // namespace c166::test::cmp
