#include "instructions/move_test.h"

namespace c166::test::movb {

void TestImmediate4ToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("E1 MOVB RL0, #15");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xA55A);

    ExecuteMove(executor, Mnemonic::kMovB, 2, std::make_unique<RegisterOperand>(kRL0),
                std::make_unique<ImmediateOperand>(15));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA50F);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }

  {
    checks.BeginCase("E1 MOVB RH0, #0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR0, 0xA55A);

    ExecuteMove(executor, Mnemonic::kMovB, 2, std::make_unique<RegisterOperand>(kRH0),
                std::make_unique<ImmediateOperand>(0));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x005A);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }
}

}  // namespace c166::test::movb
