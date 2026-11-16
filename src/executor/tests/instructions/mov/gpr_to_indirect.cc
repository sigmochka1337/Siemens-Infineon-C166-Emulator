#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestGprToIndirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("B8 MOV [R2], R3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR2, 0xFAA0);
    machine.WriteRegister(kR3, 0x1234);

    ExecuteMov(executor, 2, IndirectMemoryOperand::CreatePlain(kR2),
               std::make_unique<RegisterOperand>(kR3));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0x1234);
    checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA0);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }

  {
    checks.BeginCase("B8 ILLOPA on indirect write");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR2, 0xFAA1);
    machine.WriteRegister(kR3, 0x1234);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteMov(executor, 2, IndirectMemoryOperand::CreatePlain(kR2),
               std::make_unique<RegisterOperand>(kR3));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0xBEEF);
    checks.Equal("adjacent word", ReadWord(raw, 0xFAA2), 0x1357);
    CheckTrap(checks, machine, raw, 2, kIllegalWord);
  }
}

}  // namespace c166::test::mov
