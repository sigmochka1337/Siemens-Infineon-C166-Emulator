#include "instructions/instruction_test.h"

namespace c166::test::subc {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("32 SUBC MDH, mem via DPP0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0x0024);
    machine.WriteRegister(kDPP0, 3);
    machine.Write16(0xFA80, 0x0034);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0x3A80));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xFFEF);
    checks.Equal("source word", ReadWord(raw, 0xFA80), 0x0034);
    CheckCompleted(checks, machine, 4, kControlPsw | kCarry | kNegative);
  }

  {
    checks.BeginCase("32 SUBC R8, mem");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR8, 0x0020);
    machine.Write16(0xFA80, 0x0001);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR8),
                       std::make_unique<DirectMemoryOperand>(0xFA80));

    checks.Equal("R8", machine.ReadRegister(kR8), 0x001F);
    CheckCompleted(checks, machine, 4, kControlPsw);
  }

  {
    checks.BeginCase("32 SUBC MDH, mem raises ILLOPA on odd source");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0x1234);

    ExecuteInstruction(checks, executor, Mnemonic::kSubC, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0xFA81));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x1234);
    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::subc
