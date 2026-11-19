#include "instructions/instruction_test.h"

namespace c166::test::addc {

void TestImmediate16ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("16 ADDC MDH, #0xFFFF");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0xFFFF));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0);
    CheckCompleted(checks, machine, 4, kControlPsw | kCarry | kZero);
  }

  {
    checks.BeginCase("16 ADDC R8, #0x8000");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR8, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR8),
                       std::make_unique<ImmediateOperand>(0x8000));

    checks.Equal("R8", machine.ReadRegister(kR8), 0);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kCarry | kOverflow);
  }

  {
    checks.BeginCase("16 ADDC PSW, #1 writes the result to PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kCarry);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(1));

    CheckCompleted(checks, machine, 4, 0x2044);
  }
}

}  // namespace c166::test::addc
