#include "instructions/instruction_test.h"

namespace c166::test::subb {

void TestByteGprToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("21 SUBB RH0, RH1 signed overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x805A);
    machine.WriteRegister(kR1, 0x016B);

    ExecuteInstruction(checks, executor, Mnemonic::kSubB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH0),
                       std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x7F5A);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x016B);
    CheckCompleted(checks, machine, 2, kControlPsw | kOverflow);
  }

  {
    checks.BeginCase("21 SUBB RL0, RH1 unsigned borrow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR0, 0xA500);
    machine.WriteRegister(kR1, 0x016B);

    ExecuteInstruction(checks, executor, Mnemonic::kSubB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA5FF);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x016B);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);
  }
}

}  // namespace c166::test::subb
