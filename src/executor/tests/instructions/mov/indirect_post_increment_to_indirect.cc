#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestIndirectPostIncrementToIndirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("E8 MOV [R2], [R1+]");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);
    machine.WriteRegister(kR2, 0xFAA0);
    machine.Write16(0xFA80, 0x8000);

    ExecuteMov(executor, 2, IndirectMemoryOperand::CreatePlain(kR2),
               IndirectMemoryOperand::CreatePostIncrement(kR1));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0x8000);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFA82);
    checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA0);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("E8 ILLOPA on source");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA81);
    machine.WriteRegister(kR2, 0xFAA0);

    ExecuteMov(executor, 2, IndirectMemoryOperand::CreatePlain(kR2),
               IndirectMemoryOperand::CreatePostIncrement(kR1));

    CheckTrap(checks, machine, raw, 2, kIllegalWord);
  }

  {
    checks.BeginCase("E8 ILLOPA on destination");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);
    machine.WriteRegister(kR2, 0xFAA1);
    machine.Write16(0xFA80, 0x8000);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteMov(executor, 2, IndirectMemoryOperand::CreatePlain(kR2),
               IndirectMemoryOperand::CreatePostIncrement(kR1));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0xBEEF);
    checks.Equal("adjacent word", ReadWord(raw, 0xFAA2), 0x1357);
    CheckTrap(checks, machine, raw, 2, kIllegalWord);
  }
}

}  // namespace c166::test::mov
