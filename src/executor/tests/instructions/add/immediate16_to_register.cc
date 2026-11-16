#include "instructions/instruction_test.h"

namespace c166::test::add {

void TestImmediate16ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("06 ADD MDH, #0xFFFF");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0xFFFF));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xFFFF);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("06 ADD R8, #0x8000");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR8, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR8),
                       std::make_unique<ImmediateOperand>(0x8000));

    checks.Equal("R8", machine.ReadRegister(kR8), 0);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kCarry | kOverflow | kZero);
  }

  {
    checks.BeginCase("06 ADD PSW, #1 writes the result to PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(1));

    CheckCompleted(checks, machine, 4, 0x2041);
  }

  {
    checks.BeginCase("06 ADD SP, #2 crosses STKUN");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKUN, 0xFB20);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kSP),
                       std::make_unique<ImmediateOperand>(2));

    // A stack-limit trap from arithmetic may be serviced after later instructions (p. 3-30).
    for (int i = 0; i < 2 && machine.GetPC() != kStackUnderflowVector; ++i) {
      ExecuteInstruction(checks, executor, Mnemonic::kAdd, 2, std::nullopt,
                         std::make_unique<RegisterOperand>(kR0),
                         std::make_unique<ImmediateOperand>(0));
    }
    CheckHardwareTrap(checks, machine, kStackUnderflowVector, kStackUnderflow);
  }
}

}  // namespace c166::test::add
