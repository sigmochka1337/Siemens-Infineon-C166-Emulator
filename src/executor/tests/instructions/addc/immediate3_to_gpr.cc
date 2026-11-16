#include "instructions/instruction_test.h"

namespace c166::test::addc {

void TestImmediate3ToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("18 ADDC R0, #7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero);
    machine.WriteRegister(kR0, 0x0011);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<ImmediateOperand>(7));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x0018);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }

  {
    checks.BeginCase("18 ADDC R0, #0 preserves set Z on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry | kZero);
    machine.WriteRegister(kR0, 0xFFFF);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<ImmediateOperand>(0));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
  }

  {
    checks.BeginCase("18 ADDC R0, #0 keeps Z clear on zero");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry);
    machine.WriteRegister(kR0, 0xFFFF);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<ImmediateOperand>(0));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry);
  }
}

}  // namespace c166::test::addc
