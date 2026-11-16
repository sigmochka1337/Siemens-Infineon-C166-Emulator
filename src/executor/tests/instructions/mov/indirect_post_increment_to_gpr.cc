#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestIndirectPostIncrementToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("98 MOV R0, [R1+]");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);
    machine.Write16(0xFA80, 0x8000);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               IndirectMemoryOperand::CreatePostIncrement(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x8000);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFA82);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("98 ILLOPA on post-increment read");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA81);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               IndirectMemoryOperand::CreatePostIncrement(kR1));

    CheckTrap(checks, machine, raw, 2, kIllegalWord);
  }
}

}  // namespace c166::test::mov
