#include "instructions/instruction_test.h"

namespace c166::test::and_word {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("AND sets Z for a zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR15, 0x0000);
    machine.WriteRegister(kR2, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kAnd, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR2));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x0000);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("AND R15, R2");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 0xC531);
    machine.WriteRegister(kR2, 0xA352);
    ExecuteInstruction(checks, executor, Mnemonic::kAnd, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR2));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x8110);
    checks.Equal("R2", machine.ReadRegister(kR2), 0xA352);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("AND uses the old source for E when both operands alias");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR15, 0x8000);
    ExecuteInstruction(checks, executor, Mnemonic::kAnd, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR15));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x8000);
    CheckCompleted(checks, machine, 2, kControlPsw | kEndOfTable | kNegative);
  }
}

}  // namespace c166::test::and_word
