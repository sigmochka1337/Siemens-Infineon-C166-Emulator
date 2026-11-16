#include "instructions/instruction_test.h"

namespace c166::test::subcb {

void TestImmediate8ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("37 SUBCB MDH, #0xFF");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xA5FF);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0xFF));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x00FF);
    CheckCompleted(checks, machine, 4, kControlPsw | kCarry | kNegative);
  }

  {
    checks.BeginCase("37 SUBCB RH0, #0x80");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR0, 0x7F5A);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH0),
                       std::make_unique<ImmediateOperand>(0x80));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xFF5A);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kNegative | kCarry | kOverflow);
  }

  {
    checks.BeginCase("37 SUBCB PSW, #1 writes the result to PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero | kCarry);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(1));

    CheckCompleted(checks, machine, 4, 0x0048);
  }
}

}  // namespace c166::test::subcb
