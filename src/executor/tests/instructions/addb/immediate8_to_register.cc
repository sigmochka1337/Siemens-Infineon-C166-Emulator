#include "instructions/instruction_test.h"

namespace c166::test::addb {

void TestImmediate8ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("07 ADDB MDH, #0xFF");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xA500);

    ExecuteInstruction(checks, executor, Mnemonic::kAddB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0xFF));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x00FF);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("07 ADDB RH0, #0x80");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR0, 0x805A);

    ExecuteInstruction(checks, executor, Mnemonic::kAddB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH0),
                       std::make_unique<ImmediateOperand>(0x80));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x005A);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kCarry | kOverflow | kZero);
  }

  {
    checks.BeginCase("07 ADDB PSW, #1 writes the result to PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);

    ExecuteInstruction(checks, executor, Mnemonic::kAddB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(1));

    CheckCompleted(checks, machine, 4, 0x0041);
  }
}

}  // namespace c166::test::addb
