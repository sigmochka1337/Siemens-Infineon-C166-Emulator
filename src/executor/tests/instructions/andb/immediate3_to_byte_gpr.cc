#include "instructions/instruction_test.h"

namespace c166::test::andb {

void TestImmediate3ToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("ANDB RH7, #7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0xC55A);
    ExecuteInstruction(checks, executor, Mnemonic::kAndB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7),
                       std::make_unique<ImmediateOperand>(7));
    checks.Equal("R7", machine.ReadRegister(kR7), 0x055A);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }

  {
    checks.BeginCase("ANDB compact immediate must fit in three bits");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0xC55A);
    CheckInvalidInstruction(checks, executor, Mnemonic::kAndB, 2,
                            std::make_unique<RegisterOperand>(kRH7),
                            std::make_unique<ImmediateOperand>(8));
    checks.Equal("R7", machine.ReadRegister(kR7), 0xC55A);
  }

  {
    checks.BeginCase("ANDB compact immediate requires a GPR");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kAndB, 2,
                            std::make_unique<RegisterOperand>(kMDH),
                            std::make_unique<ImmediateOperand>(0));
  }
}

}  // namespace c166::test::andb
