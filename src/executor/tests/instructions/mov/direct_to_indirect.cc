#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestDirectToIndirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("84 MOV [R2], mem");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR2, 0xFAA0);
    machine.Write16(0xFAC0, 0x2468);

    ExecuteMov(executor, 4, IndirectMemoryOperand::CreatePlain(kR2),
               std::make_unique<DirectMemoryOperand>(0xFAC0));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0x2468);
    checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA0);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }

  {
    checks.BeginCase("84 ILLOPA on direct source");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR2, 0xFAA0);

    ExecuteMov(executor, 4, IndirectMemoryOperand::CreatePlain(kR2),
               std::make_unique<DirectMemoryOperand>(0xFAC1));

    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }

  {
    checks.BeginCase("84 ILLOPA on indirect destination");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR2, 0xFAA1);
    machine.Write16(0xFAC0, 0x2468);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteMov(executor, 4, IndirectMemoryOperand::CreatePlain(kR2),
               std::make_unique<DirectMemoryOperand>(0xFAC0));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0xBEEF);
    checks.Equal("adjacent word", ReadWord(raw, 0xFAA2), 0x1357);
    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::mov
