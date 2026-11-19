#include "instructions/instruction_test.h"

namespace c166::test::subcb {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("33 SUBCB MDH, mem via DPP0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xA524);
    machine.WriteRegister(kDPP0, 3);
    machine.Write16(0xFA80, 0x343C);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0x3A81));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x00EF);
    checks.Equal("source word", ReadWord(raw, 0xFA80), 0x343C);
    CheckCompleted(checks, machine, 4, kControlPsw | kCarry | kNegative);
  }

  {
    checks.BeginCase("33 SUBCB RH0, mem");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR0, 0x205A);
    machine.Write16(0xFA80, 0x013C);

    ExecuteInstruction(checks, executor, Mnemonic::kSubCB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH0),
                       std::make_unique<DirectMemoryOperand>(0xFA81));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x1F5A);
    CheckCompleted(checks, machine, 4, kControlPsw);
  }
}

}  // namespace c166::test::subcb
