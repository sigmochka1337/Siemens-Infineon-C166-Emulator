#include "instructions/move_test.h"

namespace c166::test::movb {

void TestIndirectPostIncrementToByteGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("99 MOVB RL0, [R1+]");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0xA55A);
  machine.WriteRegister(kR1, 0xFA81);
  machine.Write16(0xFA80, 0x427F);

  ExecuteMove(executor, Mnemonic::kMovB, 2, std::make_unique<RegisterOperand>(kRL0),
              IndirectMemoryOperand::CreatePostIncrement(kR1));

  checks.Equal("R0", machine.ReadRegister(kR0), 0xA542);
  checks.Equal("R1", machine.ReadRegister(kR1), 0xFA82);
  CheckCompleted(checks, machine, 2, kPreservedPsw);
}

}  // namespace c166::test::movb
