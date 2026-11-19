#include "instructions/move_test.h"

namespace c166::test::movb {

void TestIndirectPostIncrementToIndirect(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("E9 MOVB [R2], [R1+]");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR1, 0xFA81);
  machine.WriteRegister(kR2, 0xFAA1);
  machine.Write16(0xFA80, 0x807F);
  machine.Write16(0xFAA0, 0xBEEF);

  ExecuteMove(executor, Mnemonic::kMovB, 2, IndirectMemoryOperand::CreatePlain(kR2),
              IndirectMemoryOperand::CreatePostIncrement(kR1));

  checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x80EF);
  checks.Equal("R1", machine.ReadRegister(kR1), 0xFA82);
  checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA1);
  CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
}

}  // namespace c166::test::movb
