#include "instructions/instruction_test.h"

namespace c166::test::addb {

void TestImmediate3ToByteGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("09 ADDB RL0, #7");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0xA511);

  ExecuteInstruction(checks, executor, Mnemonic::kAddB, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kRL0),
                     std::make_unique<ImmediateOperand>(7));

  checks.Equal("R0", machine.ReadRegister(kR0), 0xA518);
  CheckCompleted(checks, machine, 2, kControlPsw);
}

}  // namespace c166::test::addb
