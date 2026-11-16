#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestIndirectWithDisplacementToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("D4 MOV R0, [R1+#4]");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);
    machine.Write16(0xFA84, 0xCAFE);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR0),
               IndirectMemoryOperand::CreateDisplacement(kR1, 4));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xCAFE);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFA80);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("D4 ILLOPA on displaced read");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR0),
               IndirectMemoryOperand::CreateDisplacement(kR1, 1));

    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::mov
