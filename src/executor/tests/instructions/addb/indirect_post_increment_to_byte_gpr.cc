#include "instructions/instruction_test.h"

namespace c166::test::addb {

void TestIndirectPostIncrementToByteGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("09 ADDB RL0, [R3+]");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0xA5FF);
  machine.WriteRegister(kR3, 0xFA81);
  machine.Write16(0xFA80, 0x013C);
  machine.Write16(0xFA82, 0x375A);

  ExecuteInstruction(checks, executor, Mnemonic::kAddB, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kRL0),
                     IndirectMemoryOperand::CreatePostIncrement(kR3));

  checks.Equal("R0", machine.ReadRegister(kR0), 0xA500);
  checks.Equal("R3", machine.ReadRegister(kR3), 0xFA82);
  CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
}

}  // namespace c166::test::addb
