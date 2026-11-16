#include "instructions/instruction_test.h"

namespace c166::test::addb {

void TestIndirectToByteGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("09 ADDB RL0, [R3]");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0xA501);
  machine.WriteRegister(kR3, 0xFA81);
  machine.Write16(0xFA80, 0x803C);

  ExecuteInstruction(checks, executor, Mnemonic::kAddB, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kRL0),
                     IndirectMemoryOperand::CreatePlain(kR3));

  checks.Equal("R0", machine.ReadRegister(kR0), 0xA581);
  checks.Equal("R3", machine.ReadRegister(kR3), 0xFA81);
  checks.Equal("source word", ReadWord(raw, 0xFA80), 0x803C);
  CheckCompleted(checks, machine, 2, kControlPsw | kEndOfTable | kNegative);
}

}  // namespace c166::test::addb
