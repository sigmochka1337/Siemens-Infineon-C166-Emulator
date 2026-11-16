#include "instructions/instruction_test.h"

namespace c166::test::sub {

void TestImmediate3ToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("28 SUB R0, #7");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0x0011);

  ExecuteInstruction(checks, executor, Mnemonic::kSub, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR0), std::make_unique<ImmediateOperand>(7));

  checks.Equal("R0", machine.ReadRegister(kR0), 0x000A);
  CheckCompleted(checks, machine, 2, kControlPsw);
}

}  // namespace c166::test::sub
