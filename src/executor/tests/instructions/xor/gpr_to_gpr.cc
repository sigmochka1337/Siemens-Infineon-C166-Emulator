#include "instructions/instruction_test.h"

namespace c166::test::xor_word {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("XOR sets Z for a zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR15, 0x0000);
    machine.WriteRegister(kR2, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR2));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x0000);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("XOR R15, R2");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 0xC531);
    machine.WriteRegister(kR2, 0xA352);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR2));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x6663);
    checks.Equal("R2", machine.ReadRegister(kR2), 0xA352);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }

  {
    checks.BeginCase("XOR uses the old source for E when both operands alias");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR15, 0x8000);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR15));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x0000);
    CheckCompleted(checks, machine, 2, kControlPsw | kEndOfTable | kZero);
  }
}

}  // namespace c166::test::xor_word
