#include "instructions/instruction_test.h"

namespace c166::test::subc {

void TestImmediate16ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("36 SUBC MDH, #0xFFFF");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xFFFF);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0xFFFF));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xFFFF);
    CheckCompleted(checks, machine, 4, kControlPsw | kCarry | kNegative);
  }

  {
    checks.BeginCase("36 SUBC R8, #0x8000");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR8, 0x7FFF);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR8),
                       std::make_unique<ImmediateOperand>(0x8000));

    checks.Equal("R8", machine.ReadRegister(kR8), 0xFFFF);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kNegative | kCarry | kOverflow);
  }

  {
    checks.BeginCase("36 SUBC PSW, #1 writes the result to PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw | kZero | kCarry);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(1));

    CheckCompleted(checks, machine, 4, 0x2048);
  }
}

}  // namespace c166::test::subc
