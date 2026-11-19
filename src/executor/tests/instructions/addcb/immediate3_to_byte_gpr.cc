#include "instructions/instruction_test.h"

namespace c166::test::addcb {

void TestImmediate3ToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("19 ADDCB RL0, #7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero);
    machine.WriteRegister(kR0, 0xA511);

    ExecuteInstruction(checks, executor, Mnemonic::kAddCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA518);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }

  {
    checks.BeginCase("19 ADDCB RL0, #0 preserves set Z on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0xA5FF);

    ExecuteInstruction(checks, executor, Mnemonic::kAddCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<ImmediateOperand>(0));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA500);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
  }

  {
    checks.BeginCase("19 ADDCB RL0, #0 keeps Z clear on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry);
    machine.WriteRegister(kR0, 0xA5FF);

    ExecuteInstruction(checks, executor, Mnemonic::kAddCB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRL0),
                       std::make_unique<ImmediateOperand>(0));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xA500);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry);
  }
}

}  // namespace c166::test::addcb
