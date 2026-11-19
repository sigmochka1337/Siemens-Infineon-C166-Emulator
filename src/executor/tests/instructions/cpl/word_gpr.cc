#include "instructions/instruction_test.h"

namespace c166::test::cpl {

void TestWordGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("CPL R15 with input 0x8000");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 0x8000);
    ExecuteInstruction(checks, executor, Mnemonic::kCpl, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x7FFF);
    CheckCompleted(checks, machine, 2, kControlPsw | kEndOfTable);
  }

  {
    checks.BeginCase("CPL R15 with input 0x0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 0x0000);
    ExecuteInstruction(checks, executor, Mnemonic::kCpl, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15));
    checks.Equal("R15", machine.ReadRegister(kR15), 0xFFFF);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("CPL R15 with input 0xffff");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 0xFFFF);
    ExecuteInstruction(checks, executor, Mnemonic::kCpl, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x0000);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("CPL requires a GPR");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kCpl, 2,
                            std::make_unique<RegisterOperand>(kMDH));
  }
}

}  // namespace c166::test::cpl
