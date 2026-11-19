#include "instructions/instruction_test.h"

namespace c166::test::addc {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("10 ADDC R0, R1 signed overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x7FFF);
    machine.WriteRegister(kR1, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x8000);
    checks.Equal("R1", machine.ReadRegister(kR1), 0);
    CheckCompleted(checks, machine, 2, kControlPsw | kOverflow | kNegative);
  }

  {
    checks.BeginCase("10 ADDC R0, R1 carry with zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0x8000);
    machine.WriteRegister(kR1, 0x7FFF);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x7FFF);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
  }
}

}  // namespace c166::test::addc
