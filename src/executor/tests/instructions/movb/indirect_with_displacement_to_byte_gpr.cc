#include "instructions/move_test.h"

namespace c166::test::movb {

void TestIndirectWithDisplacementToByteGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("F4 MOVB RH0, [R1 + #4]");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0xA55A);
  machine.WriteRegister(kR1, 0xFA7D);
  machine.Write16(0xFA80, 0xFF7F);

  ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<RegisterOperand>(kRH0),
              IndirectMemoryOperand::CreateDisplacement(kR1, 4));

  checks.Equal("R0", machine.ReadRegister(kR0), 0xFF5A);
  checks.Equal("R1", machine.ReadRegister(kR1), 0xFA7D);
  CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
}

}  // namespace c166::test::movb
