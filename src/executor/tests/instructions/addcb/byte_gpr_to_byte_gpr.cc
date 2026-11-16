#include "instructions/instruction_test.h"

namespace c166::test::addcb {

void TestByteGprToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("11 ADDCB RH0, RH1 signed overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x7F5A);
    machine.WriteRegister(kR1, 0x006B);

    ExecuteInstruction(checks, executor, Mnemonic::kAddCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH0),
                       std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x805A);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x006B);
    CheckCompleted(checks, machine, 2, kControlPsw | kOverflow | kNegative);
  }

  {
    checks.BeginCase("11 ADDCB RL0, RH1 carry with zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0xA580);
    machine.WriteRegister(kR1, 0x7F6B);

    ExecuteInstruction(checks, executor, Mnemonic::kAddCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA500);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x7F6B);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
  }
}

}  // namespace c166::test::addcb
