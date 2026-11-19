#include "instructions/instruction_test.h"

namespace c166::test::subc {

void TestImmediate3ToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("38 SUBC R0, #7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero);
    machine.WriteRegister(kR0, 0x0011);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x000A);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }

  {
    checks.BeginCase("38 SUBC R0, #7 preserves set Z on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0x0008);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("38 SUBC R0, #7 keeps Z clear on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry);
    machine.WriteRegister(kR0, 0x0008);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }
}

}  // namespace c166::test::subc
