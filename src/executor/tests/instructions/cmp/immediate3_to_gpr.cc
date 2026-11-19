#include "instructions/instruction_test.h"

namespace c166::test::cmp {

void TestImmediate3ToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMP compact immediate");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR4, 0);

  ExecuteInstruction(checks, executor, Mnemonic::kCmp, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR4), std::make_unique<ImmediateOperand>(0));

  checks.Equal("left operand", machine.ReadRegister(kR4), 0);

  CheckCompleted(checks, machine, 2, kControlPsw | kZero);
}

}  // namespace c166::test::cmp
