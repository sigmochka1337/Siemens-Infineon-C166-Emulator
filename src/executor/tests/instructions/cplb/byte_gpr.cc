#include "instructions/instruction_test.h"

namespace c166::test::cplb {

void TestByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("CPLB RH7 with input 0x80");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0x806A);
    ExecuteInstruction(checks, executor, Mnemonic::kCplB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7));
    checks.Equal("R7", machine.ReadRegister(kR7), 0x7F6A);
    CheckCompleted(checks, machine, 2, kControlPsw | kEndOfTable);
  }

  {
    checks.BeginCase("CPLB RH7 with input 0x0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0x006A);
    ExecuteInstruction(checks, executor, Mnemonic::kCplB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7));
    checks.Equal("R7", machine.ReadRegister(kR7), 0xFF6A);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("CPLB RH7 with input 0xff");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0xFF6A);
    ExecuteInstruction(checks, executor, Mnemonic::kCplB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7));
    checks.Equal("R7", machine.ReadRegister(kR7), 0x006A);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("CPLB requires a GPR");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kCplB, 2,
                            std::make_unique<RegisterOperand>(kMDH));
  }
}

}  // namespace c166::test::cplb
