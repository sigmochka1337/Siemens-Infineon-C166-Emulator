#include "instructions/instruction_test.h"

namespace c166::test::or_word {

void TestIndirectPostIncrementToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("OR R15, [R3+] via DPP1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 5);
    machine.WriteRegister(kR3, 0x4030);
    machine.WriteRegister(kR15, 0xC531);
    machine.Write16(0x14030, 0xA352);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       IndirectMemoryOperand::CreatePostIncrement(kR3));
    checks.Equal("R15", machine.ReadRegister(kR15), 0xE773);
    checks.Equal("R3", machine.ReadRegister(kR3), 0x4032);
    checks.Equal("source word", ReadWord(raw, 0x14030), 0xA352);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("OR indirect encoding only selects R0 through R3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kOr, 2,
                            std::make_unique<RegisterOperand>(kR15),
                            IndirectMemoryOperand::CreatePostIncrement(kR4));
  }

  {
    checks.BeginCase("OR traps on an unaligned indirect word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR3, 0xFA81);
    machine.WriteRegister(kR15, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       IndirectMemoryOperand::CreatePostIncrement(kR3));
    checks.Equal("R15", machine.ReadRegister(kR15), 0xC531);
    checks.Equal("R3", machine.ReadRegister(kR3), 0xFA81);
    CheckTrap(checks, machine, raw, 2, kIllegalWord);
  }

  {
    checks.BeginCase("OR R3, [R3+] writes the result after incrementing R3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR3, 0xFA80);
    machine.Write16(0xFA80, 0x003F);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR3),
                       IndirectMemoryOperand::CreatePostIncrement(kR3));
    checks.Equal("R3", machine.ReadRegister(kR3), 0xFABF);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }
}

}  // namespace c166::test::or_word
