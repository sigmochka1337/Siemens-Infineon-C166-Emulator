#include "instructions/instruction_test.h"

namespace c166::test::andb {

void TestByteGprToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("ANDB sets Z for a zero result");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR7, 0x005A);
    machine.WriteRegister(kRL2, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kAndB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7),
                       std::make_unique<RegisterOperand>(kRL2));
    checks.Equal("R7", machine.ReadRegister(kR7), 0x005A);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("ANDB RH7, RL2");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, 0xC55A);
    machine.WriteRegister(kRL2, 0x00A3);
    ExecuteInstruction(checks, executor, Mnemonic::kAndB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7),
                       std::make_unique<RegisterOperand>(kRL2));
    checks.Equal("R7", machine.ReadRegister(kR7), 0x815A);
    checks.Equal("RL2", machine.ReadRegister(kRL2), 0x00A3);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("ANDB uses the old source for E when both operands alias");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR7, 0x805A);
    ExecuteInstruction(checks, executor, Mnemonic::kAndB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7),
                       std::make_unique<RegisterOperand>(kRH7));
    checks.Equal("R7", machine.ReadRegister(kR7), 0x805A);
    CheckCompleted(checks, machine, 2, kControlPsw | kEndOfTable | kNegative);
  }

  {
    checks.BeginCase("ANDB rejects word GPRs in a byte GPR pair");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kAndB, 2,
                            std::make_unique<RegisterOperand>(kR2),
                            std::make_unique<RegisterOperand>(kRL7));
    CheckInvalidInstruction(checks, executor, Mnemonic::kAndB, 2,
                            std::make_unique<RegisterOperand>(kRH0),
                            std::make_unique<RegisterOperand>(kR2));
  }
}

}  // namespace c166::test::andb
