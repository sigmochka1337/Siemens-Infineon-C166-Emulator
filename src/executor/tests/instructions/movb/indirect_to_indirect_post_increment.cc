#include "instructions/move_test.h"

namespace c166::test::movb {

void TestIndirectToIndirectPostIncrement(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("D9 MOVB [R2+], [R1]");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR1, 0xFA81);
  machine.WriteRegister(kR2, 0xFAA1);
  machine.Write16(0xFA80, 0x557F);
  machine.Write16(0xFAA0, 0xBEEF);
  machine.Write16(0xFAA2, 0x1357);

  ExecuteMove(executor, Mnemonic::kMovB, 2, IndirectMemoryOperand::CreatePostIncrement(kR2),
              IndirectMemoryOperand::CreatePlain(kR1));

  checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x55EF);
  checks.Equal("next word", ReadWord(raw, 0xFAA2), 0x1357);
  checks.Equal("R1", machine.ReadRegister(kR1), 0xFA81);
  checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA2);
  CheckCompleted(checks, machine, 2, kPreservedPsw);
}

}  // namespace c166::test::movb
