#include "instructions/move_test.h"

namespace c166::test::movbz {

void TestByteGprToWordGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("C0 MOVBZ R0, RH1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw | kEndOfTable | kZero);
    machine.WriteRegister(kR0, 0xA55A);
    machine.WriteRegister(kR1, 0x807F);

    ExecuteMove(executor, Mnemonic::kMovBZ, 2, std::make_unique<RegisterOperand>(kR0),
                std::make_unique<RegisterOperand>(kRH1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x0080);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x807F);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }

  {
    checks.BeginCase("C0 MOVBZ R0, RL1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xA55A);
    machine.WriteRegister(kR1, 0x807F);

    ExecuteMove(executor, Mnemonic::kMovBZ, 2, std::make_unique<RegisterOperand>(kR0),
                std::make_unique<RegisterOperand>(kRL1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x007F);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x807F);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }
}

}  // namespace c166::test::movbz
