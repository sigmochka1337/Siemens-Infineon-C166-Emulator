#include "instructions/instruction_test.h"

namespace c166::test::neg {

void TestWordGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("81 NEG R15 when the operand is zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR15, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kNeg, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15));

    checks.Equal("R15", machine.ReadRegister(kR15), 0);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("81 NEG R15 when the operand is one");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 1);

    ExecuteInstruction(checks, executor, Mnemonic::kNeg, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15));

    checks.Equal("R15", machine.ReadRegister(kR15), 0xFFFF);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);
  }

  {
    checks.BeginCase("81 NEG R15 when the operand is minus one");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR15, 0xFFFF);

    ExecuteInstruction(checks, executor, Mnemonic::kNeg, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15));

    checks.Equal("R15", machine.ReadRegister(kR15), 1);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry);
  }

  {
    checks.BeginCase("81 NEG R15 when the operand is the minimum signed word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero);
    machine.WriteRegister(kR15, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kNeg, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15));

    checks.Equal("R15", machine.ReadRegister(kR15), 0x8000);
    CheckCompleted(checks, machine, 2, kControlPsw | kEndOfTable | kOverflow | kCarry | kNegative);
  }
}

}  // namespace c166::test::neg
