#include "instructions/move_test.h"

namespace c166::test::movb {

void TestByteGprToIndirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("B9 MOVB [R2], RL3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR2, 0xFAA1);
    machine.WriteRegister(kR3, 0x127F);
    machine.Write16(0xFAA0, 0xBEEF);

    ExecuteMove(executor, Mnemonic::kMovB, 2, IndirectMemoryOperand::CreatePlain(kR2),
                std::make_unique<RegisterOperand>(kRL3));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x7FEF);
    checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA1);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }

  {
    checks.BeginCase("B9 MOVB [R2], RL3 writes the low byte of MDH");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    // A byte write to a word SFR clears the other byte (pp. 3-5 and 3-6).
    machine.WriteRegister(kR2, 0xFE0C);
    machine.WriteRegister(kR3, 0x127F);
    machine.WriteRegister(kMDH, 0xA55A);

    ExecuteMove(executor, Mnemonic::kMovB, 2, IndirectMemoryOperand::CreatePlain(kR2),
                std::make_unique<RegisterOperand>(kRL3));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x007F);
    checks.Equal("R2", machine.ReadRegister(kR2), 0xFE0C);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }
}

}  // namespace c166::test::movb
