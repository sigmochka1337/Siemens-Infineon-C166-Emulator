#include "instructions/move_test.h"

namespace c166::test::movb {

void TestIndirectToByteGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("A9 MOVB RH0, [R1]");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0xA55A);
  machine.WriteRegister(kR1, 0xFA81);
  machine.Write16(0xFA80, 0x007F);

  ExecuteMove(executor, Mnemonic::kMovB, 2, std::make_unique<RegisterOperand>(kRH0),
              IndirectMemoryOperand::CreatePlain(kR1));

  checks.Equal("R0", machine.ReadRegister(kR0), 0x005A);
  checks.Equal("R1", machine.ReadRegister(kR1), 0xFA81);
  CheckCompleted(checks, machine, 2, kPreservedPsw | kZero);
}

}  // namespace c166::test::movb
