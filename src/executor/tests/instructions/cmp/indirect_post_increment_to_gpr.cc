#include "instructions/instruction_test.h"

namespace c166::test::cmp {

void TestIndirectPostIncrementToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMP indirect with post-increment");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR4, 0);
  machine.WriteRegister(kR3, 0xFA80);
  machine.Write16(0xFA80, 1);

  ExecuteInstruction(checks, executor, Mnemonic::kCmp, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR4),
                     IndirectMemoryOperand::CreatePostIncrement(kR3));

  checks.Equal("left operand", machine.ReadRegister(kR4), 0);
  checks.Equal("pointer", machine.ReadRegister(kR3), 0xFA82);
  CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);

  checks.BeginCase("CMP leaves an odd source pointer intact on ILLOPA");
  Prepare(machine);
  machine.WriteRegister(kR3, 0xFA81);
  ExecuteInstruction(checks, executor, Mnemonic::kCmp, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR4),
                     IndirectMemoryOperand::CreatePostIncrement(kR3));
  CheckTrap(checks, machine, raw, 2, kIllegalWord);
  checks.Equal("pointer", machine.ReadRegister(kR3), 0xFA81);
}

}  // namespace c166::test::cmp
