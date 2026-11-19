#include "instructions/move_test.h"

namespace c166::test::movbs {

void TestByteGprToWordGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("D0 MOVBS R0, RH1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw | kEndOfTable | kZero);
    machine.WriteRegister(kR0, 0xA55A);
    machine.WriteRegister(kR1, 0x807F);

    ExecuteMove(executor, Mnemonic::kMovBS, 2, std::make_unique<RegisterOperand>(kR0),
                std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xFF80);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x807F);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("D0 MOVBS R0, RL1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xA55A);
    machine.WriteRegister(kR1, 0x807F);

    ExecuteMove(executor, Mnemonic::kMovBS, 2, std::make_unique<RegisterOperand>(kR0),
                std::make_unique<RegisterOperand>(kRL1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x007F);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x807F);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }
}

}  // namespace c166::test::movbs
