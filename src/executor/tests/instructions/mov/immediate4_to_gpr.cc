#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestImmediate4ToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("E0 MOV R0, #15");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<ImmediateOperand>(15));

    checks.Equal("R0", machine.ReadRegister(kR0), 15);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }

  {
    checks.BeginCase("E0 MOV R0, #0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xFFFF);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<ImmediateOperand>(0));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kZero);
  }
}

}  // namespace c166::test::mov
