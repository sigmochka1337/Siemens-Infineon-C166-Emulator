#include "instructions/instruction_test.h"

namespace c166::test::sub {

void TestImmediate16ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("26 SUB MDH, #0xFFFF");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kSub, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0xFFFF));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x0001);
    CheckCompleted(checks, machine, 4, kControlPsw | kCarry);
  }

  {
    checks.BeginCase("26 SUB R8, #0x8000");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR8, 0x7FFF);

    ExecuteInstruction(checks, executor, Mnemonic::kSub, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR8),
                       std::make_unique<ImmediateOperand>(0x8000));

    checks.Equal("R8", machine.ReadRegister(kR8), 0xFFFF);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kNegative | kCarry | kOverflow);
  }

  {
    checks.BeginCase("26 SUB PSW, #1 writes the result to PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero);

    ExecuteInstruction(checks, executor, Mnemonic::kSub, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(1));

    CheckCompleted(checks, machine, 4, 0x2047);
  }

  {
    checks.BeginCase("26 SUB SP, #2 crosses STKOV");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKOV, 0xFB20);

    ExecuteInstruction(checks, executor, Mnemonic::kSub, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kSP),
                       std::make_unique<ImmediateOperand>(2));

    // A stack-limit trap from arithmetic may be serviced after later instructions (p. 3-30).
    for (int i = 0; i < 2 && machine.GetPC() != kStackOverflowVector; ++i) {
      ExecuteInstruction(checks, executor, Mnemonic::kSub, 2, std::nullopt,
                         std::make_unique<RegisterOperand>(kR0),
                         std::make_unique<ImmediateOperand>(0));
    }
    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
  }
}

}  // namespace c166::test::sub
