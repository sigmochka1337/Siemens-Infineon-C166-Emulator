#include "instructions/instruction_test.h"

namespace c166::test::subcb {

void TestByteGprToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("31 SUBCB RH0, RH1 signed overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x805A);
    machine.WriteRegister(kR1, 0x006B);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH0),
                       std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x7F5A);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x006B);
    CheckCompleted(checks, machine, 2, kControlPsw | kOverflow);
  }

  {
    checks.BeginCase("31 SUBCB RL0, RH1 borrow with zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0xA500);
    machine.WriteRegister(kR1, 0xFF6B);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA500);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFF6B);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
  }
}

}  // namespace c166::test::subcb
