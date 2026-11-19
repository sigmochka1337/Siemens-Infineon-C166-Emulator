#include "instructions/move_test.h"

namespace c166::test::movbs {

void TestRegisterToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("D5 MOVBS mem, MDH");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0x7FFF);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteMove(executor, Mnemonic::kMovBS, 4, std::make_unique<DirectMemoryOperand>(0xFAA0),
                std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0xFFFF);
    checks.Equal("next word", ReadWord(raw, 0xFAA2), 0x1357);
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x7FFF);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("D5 MOVBS mem, RH3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR3, 0x007F);
    machine.Write16(0xFAA0, 0xBEEF);

    ExecuteMove(executor, Mnemonic::kMovBS, 4, std::make_unique<DirectMemoryOperand>(0xFAA0),
                std::make_unique<RegisterOperand>(kRH3));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0);
    checks.Equal("R3", machine.ReadRegister(kR3), 0x007F);
    CheckCompleted(checks, machine, 4, kControlPsw | kZero);
  }

  {
    checks.BeginCase("D5 MOVBS raises ILLOPA on odd word destination");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR3, 0x807F);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteMove(executor, Mnemonic::kMovBS, 4, std::make_unique<DirectMemoryOperand>(0xFAA1),
                std::make_unique<RegisterOperand>(kRH3));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0xBEEF);
    checks.Equal("next word", ReadWord(raw, 0xFAA2), 0x1357);
    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::movbs
