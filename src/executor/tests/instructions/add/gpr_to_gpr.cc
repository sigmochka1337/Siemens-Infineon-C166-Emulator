#include "instructions/instruction_test.h"

namespace c166::test::add {

void TestGprToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("00 ADD R0, R1 signed overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x7FFF);
    machine.WriteRegister(kR1, 0x0001);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x8000);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x0001);
    CheckCompleted(checks, machine, 2, kControlPsw | kOverflow | kNegative);
  }

  {
    checks.BeginCase("00 ADD R0, R1 carry and overflow with zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR0, 0x8000);
    machine.WriteRegister(kR1, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x8000);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero | kOverflow | kEndOfTable);
  }
}

}  // namespace c166::test::add
