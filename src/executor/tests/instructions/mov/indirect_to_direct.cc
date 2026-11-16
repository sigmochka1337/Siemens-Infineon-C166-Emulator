#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestIndirectToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("94 MOV mem, [R1]");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);
    machine.Write16(0xFA80, 0x8000);

    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0xFAE0),
               IndirectMemoryOperand::CreatePlain(kR1));

    checks.Equal("destination", ReadWord(raw, 0xFAE0), 0x8000);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFA80);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("94 ILLOPA on indirect source");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA81);

    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0xFAA0),
               IndirectMemoryOperand::CreatePlain(kR1));

    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }

  {
    checks.BeginCase("94 ILLOPA on direct destination");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);
    machine.Write16(0xFA80, 0x8000);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0xFAA1),
               IndirectMemoryOperand::CreatePlain(kR1));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0xBEEF);
    checks.Equal("adjacent word", ReadWord(raw, 0xFAA2), 0x1357);
    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::mov
