#include "instructions/instruction_test.h"

namespace c166::test::add {

void TestImmediate3ToGpr(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("08 ADD R0, #7");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(kR0, 0x0011);

  ExecuteInstruction(checks, executor, Mnemonic::kAdd, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR0), std::make_unique<ImmediateOperand>(7));

  checks.Equal("R0", machine.ReadRegister(kR0), 0x0018);
  CheckCompleted(checks, machine, 2, kControlPsw);

  {
    checks.BeginCase("ADD rejects #8 in its compact form");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x1234);
    CheckInvalidInstruction(checks, executor, Mnemonic::kAdd, 2,
                            std::make_unique<RegisterOperand>(kR0),
                            std::make_unique<ImmediateOperand>(8));
    checks.Equal("R0", machine.ReadRegister(kR0), 0x1234);
    CheckCompleted(checks, machine, kStart, kInitialPsw);
  }
}

}  // namespace c166::test::add
