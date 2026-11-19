#include "instructions/move_test.h"

namespace c166::test::movb {

void TestIndirectToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("B4 MOVB mem, [R1]");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA81);
    machine.Write16(0xFA80, 0x007F);
    machine.Write16(0xFAA0, 0xBEEF);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<DirectMemoryOperand>(0xFAA1),
                IndirectMemoryOperand::CreatePlain(kR1));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x00EF);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFA81);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kZero);
  }

  {
    checks.BeginCase("B4 MOVB mem, [R1] writes the high byte of MDH");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    // Long addressing can write the high byte of a word SFR (pp. 3-5 and 3-6).
    machine.WriteRegister(kR1, 0xFA81);
    machine.Write16(0xFA80, 0x427F);
    machine.WriteRegister(kMDH, 0xA55A);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<DirectMemoryOperand>(0xFE0D),
                IndirectMemoryOperand::CreatePlain(kR1));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x4200);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFA81);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }
}

}  // namespace c166::test::movb
