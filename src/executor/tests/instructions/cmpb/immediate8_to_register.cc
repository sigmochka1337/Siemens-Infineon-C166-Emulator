#include "instructions/instruction_test.h"

namespace c166::test::cmpb {

void TestImmediate8ToRegister(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMPB minimum signed source");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kMDH, 0);
  ExecuteInstruction(checks, executor, Mnemonic::kCmpB, 4, std::nullopt,
                     std::make_unique<RegisterOperand>(kMDH),
                     std::make_unique<ImmediateOperand>(0x80));

  checks.Equal("left operand", machine.ReadRegister(kMDH), 0);

  CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kCarry | kNegative | kOverflow);
}

}  // namespace c166::test::cmpb
