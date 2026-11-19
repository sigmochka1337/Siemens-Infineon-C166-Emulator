#include "instructions/instruction_test.h"

namespace c166::test::xor_word {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("XOR MDH, mem via DPP1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 5);
    machine.WriteRegister(kMDH, 0xC531);
    machine.Write16(0x14030, 0xA352);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0x4030));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x6663);
    checks.Equal("addressed word", ReadWord(raw, 0x14030), 0xA352);
    CheckCompleted(checks, machine, 4, kControlPsw);
  }

  {
    checks.BeginCase("XOR ILLOPA on odd direct source");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0xFA81));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xC531);
    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::xor_word
