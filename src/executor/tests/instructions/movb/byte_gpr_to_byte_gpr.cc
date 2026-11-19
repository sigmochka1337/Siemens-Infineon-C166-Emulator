#include "instructions/move_test.h"

namespace c166::test::movb {

void TestByteGprToByteGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("F1 MOVB RL0, RH1");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine, kPreservedPsw);
  machine.WriteRegister(kR0, 0xA55A);
  machine.WriteRegister(kR1, 0x807F);

  ExecuteMove(executor, Mnemonic::kMovB, 2, std::make_unique<RegisterOperand>(kRL0),
              std::make_unique<RegisterOperand>(kRH1));

  checks.Equal("R0", machine.ReadRegister(kR0), 0xA580);
  checks.Equal("R1", machine.ReadRegister(kR1), 0x807F);
  CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
}

}  // namespace c166::test::movb
