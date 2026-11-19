#include "instructions/instruction_test.h"

namespace c166::test::cmpb {

void TestImmediate3ToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMPB compact immediate");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kRH4, 0);

  ExecuteInstruction(checks, executor, Mnemonic::kCmpB, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kRH4),
                     std::make_unique<ImmediateOperand>(0));

  checks.Equal("left operand", machine.ReadRegister(kRH4), 0);

  CheckCompleted(checks, machine, 2, kControlPsw | kZero);
}

}  // namespace c166::test::cmpb
