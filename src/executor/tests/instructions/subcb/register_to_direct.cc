#include "instructions/instruction_test.h"

namespace c166::test::subcb {

void TestRegisterToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("35 SUBCB mem, MDH");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xA503);
    machine.Write16(0xFAA0, 0x233C);
    machine.Write16(0xFAA2, 0xBEEF);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFAA1),
                       std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x1F3C);
    checks.Equal("next word", ReadWord(raw, 0xFAA2), 0xBEEF);
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xA503);
    CheckCompleted(checks, machine, 4, kControlPsw);
  }

  {
    checks.BeginCase("35 SUBCB mem, RH1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR1, 0x016B);
    machine.Write16(0xFAA0, 0x803C);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFAA1),
                       std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x7F3C);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x016B);
    CheckCompleted(checks, machine, 4, kControlPsw | kOverflow);
  }
}

}  // namespace c166::test::subcb
