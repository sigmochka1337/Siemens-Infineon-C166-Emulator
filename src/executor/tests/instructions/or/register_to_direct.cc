#include "instructions/instruction_test.h"

namespace c166::test::or_word {

void TestRegisterToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("OR mem, MDH via DPP1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 5);
    machine.WriteRegister(kMDH, 0xC531);
    machine.Write16(0x14030, 0xA352);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0x4030),
                       std::make_unique<RegisterOperand>(kMDH));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xC531);
    checks.Equal("addressed word", ReadWord(raw, 0x14030), 0xE773);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("OR ILLOPA on odd direct destination");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFA81),
                       std::make_unique<RegisterOperand>(kMDH));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xC531);
    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::or_word
