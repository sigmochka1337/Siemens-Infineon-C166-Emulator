#include "instructions/instruction_test.h"

namespace c166::test::orb {

void TestImmediate3ToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("ORB RH7, #7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0xC55A);
    ExecuteInstruction(checks, executor, Mnemonic::kOrB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7),
                       std::make_unique<ImmediateOperand>(7));
    checks.Equal("R7", machine.ReadRegister(kR7), 0xC75A);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("ORB compact immediate must fit in three bits");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0xC55A);
    CheckInvalidInstruction(checks, executor, Mnemonic::kOrB, 2,
                            std::make_unique<RegisterOperand>(kRH7),
                            std::make_unique<ImmediateOperand>(8));
    checks.Equal("R7", machine.ReadRegister(kR7), 0xC55A);
  }

  {
    checks.BeginCase("ORB compact immediate requires a GPR");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kOrB, 2,
                            std::make_unique<RegisterOperand>(kMDH),
                            std::make_unique<ImmediateOperand>(0));
  }
}

}  // namespace c166::test::orb
