#include "instructions/move_test.h"

namespace c166::test::movb {

void TestRegisterToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("F7 MOVB mem, MDH");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0x7F80);
    machine.Write16(0xFAA0, 0xBEEF);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<DirectMemoryOperand>(0xFAA1),
                std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x80EF);
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x7F80);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("F7 MOVB mem, RH3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR3, 0x427F);
    machine.Write16(0xFAA0, 0xBEEF);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<DirectMemoryOperand>(0xFAA1),
                std::make_unique<RegisterOperand>(kRH3));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x42EF);
    checks.Equal("R3", machine.ReadRegister(kR3), 0x427F);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }
}

}  // namespace c166::test::movb
