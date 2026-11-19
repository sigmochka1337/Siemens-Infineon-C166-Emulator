#include "instructions/instruction_test.h"

namespace c166::test::sub {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("20 SUB R0, R1 signed overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x8000);
    machine.WriteRegister(kR1, 0x0001);

    ExecuteInstruction(checks, executor, Mnemonic::kSub, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x7FFF);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x0001);
    CheckCompleted(checks, machine, 2, kControlPsw | kOverflow);
  }

  {
    checks.BeginCase("20 SUB R0, R1 unsigned borrow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR0, 0);
    machine.WriteRegister(kR1, 0x0001);

    ExecuteInstruction(checks, executor, Mnemonic::kSub, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xFFFF);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x0001);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);
  }
}

}  // namespace c166::test::sub
