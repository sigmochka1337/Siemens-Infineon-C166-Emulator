#include "instructions/instruction_test.h"

namespace c166::test::cmpb {

void TestIndirectToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("CMPB indirect");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kRH4, 0);
  machine.WriteRegister(kR3, 0xFA80);
  machine.Write16(0xFA80, 1);

  ExecuteInstruction(checks, executor, Mnemonic::kCmpB, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kRH4),
                     IndirectMemoryOperand::CreatePlain(kR3));

  checks.Equal("left operand", machine.ReadRegister(kRH4), 0);
  checks.Equal("pointer", machine.ReadRegister(kR3), 0xFA80);
  CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kNegative);
}

}  // namespace c166::test::cmpb
