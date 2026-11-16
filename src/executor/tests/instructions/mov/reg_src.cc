#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestRegisterSource(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("F0 MOV R0, R3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR3, 0x1234);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<RegisterOperand>(kR3));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x1234);
    checks.Equal("R3", machine.ReadRegister(kR3), 0x1234);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }
}

}  // namespace c166::test::mov
