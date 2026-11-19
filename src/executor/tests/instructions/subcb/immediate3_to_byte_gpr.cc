#include "instructions/instruction_test.h"

namespace c166::test::subcb {

void TestImmediate3ToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("39 SUBCB RL0, #7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero);
    machine.WriteRegister(kR0, 0xA511);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA50A);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }

  {
    checks.BeginCase("39 SUBCB RL0, #7 preserves set Z on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0xA508);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA500);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("39 SUBCB RL0, #7 keeps Z clear on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry);
    machine.WriteRegister(kR0, 0xA508);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA500);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }
}

}  // namespace c166::test::subcb
