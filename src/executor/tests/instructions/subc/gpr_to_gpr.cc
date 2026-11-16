#include "instructions/instruction_test.h"

namespace c166::test::subc {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("30 SUBC R0, R1 signed overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x8000);
    machine.WriteRegister(kR1, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x7FFF);
    checks.Equal("R1", machine.ReadRegister(kR1), 0);
    CheckCompleted(checks, machine, 2, kControlPsw | kOverflow);
  }

  {
    checks.BeginCase("30 SUBC R0, R1 borrow with zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0);
    machine.WriteRegister(kR1, 0xFFFF);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFFFF);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
  }
}

}  // namespace c166::test::subc
