#include "instructions/move_test.h"

namespace c166::test::movb {

void TestDirectToIndirect(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("A4 MOVB [R2], mem");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR2, 0xFAA1);
  machine.Write16(0xFA80, 0x427F);
  machine.Write16(0xFAA0, 0xBEEF);

  ExecuteMove(executor, Mnemonic::kMovB, 4, IndirectMemoryOperand::CreatePlain(kR2),
              std::make_unique<DirectMemoryOperand>(0xFA81));

  checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x42EF);
  checks.Equal("R2", machine.ReadRegister(kR2), 0xFAA1);
  CheckCompleted(checks, machine, 4, kPreservedPsw);
}

}  // namespace c166::test::movb
