#include "instructions/instruction_test.h"

namespace c166::test::cmpb {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMPB register pair");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kRH4, 0);
  machine.WriteRegister(kRL6, 1);
  ExecuteInstruction(checks, executor, Mnemonic::kCmpB, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kRH4),
                     std::make_unique<RegisterOperand>(kRL6));

  checks.Equal("left operand", machine.ReadRegister(kRH4), 0);

  CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);
}

}  // namespace c166::test::cmpb
