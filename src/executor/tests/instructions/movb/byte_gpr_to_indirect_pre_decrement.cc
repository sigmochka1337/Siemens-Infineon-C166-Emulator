#include "instructions/move_test.h"

namespace c166::test::movb {

void TestByteGprToIndirectPreDecrement(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("89 MOVB [-R2], RH3");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR2, 0xFAA2);
  machine.WriteRegister(kR3, 0x807F);
  machine.Write16(0xFAA0, 0xBEEF);
  machine.Write16(0xFAA2, 0x1357);

  ExecuteMove(executor, Mnemonic::kMovB, 2, IndirectMemoryOperand::CreatePreDecrement(kR2),
              std::make_unique<RegisterOperand>(kRH3));

  checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x80EF);
  checks.Equal("next word", ReadWord(raw, 0xFAA2), 0x1357);
  checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA1);
  CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
}

}  // namespace c166::test::movb
