#include "instructions/move_test.h"

namespace c166::test::movb {

void TestByteGprToIndirectWithDisplacement(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("E4 MOVB [R2 + #4], RL3");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR2, 0xFA9D);
  machine.WriteRegister(kR3, 0x1281);
  machine.Write16(0xFAA0, 0xBEEF);

  ExecuteMove(executor, Mnemonic::kMovB, 4, IndirectMemoryOperand::CreateDisplacement(kR2, 4),
              std::make_unique<RegisterOperand>(kRL3));

  checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x81EF);
  checks.Equal("R2", machine.ReadRegister(kR2), 0xFA9D);
  CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
}

}  // namespace c166::test::movb
