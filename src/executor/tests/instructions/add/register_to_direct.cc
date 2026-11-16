#include "instructions/instruction_test.h"

namespace c166::test::add {

void TestRegisterToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("04 ADD mem, MDH");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0x0003);
    machine.Write16(0xFAA0, 0x0023);
    machine.Write16(0xFAA2, 0xBEEF);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFAA0),
                       std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0x0026);
    checks.Equal("next word", ReadWord(raw, 0xFAA2), 0xBEEF);
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x0003);
    CheckCompleted(checks, machine, 4, kControlPsw);
  }

  {
    checks.BeginCase("04 ADD mem, R1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kR1, 0x8000);
    machine.Write16(0xFAA0, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFAA0),
                       std::make_unique<RegisterOperand>(kR1));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0);
    checks.Equal("R1", machine.ReadRegister(kR1), 0x8000);
    CheckCompleted(checks, machine, 4, kControlPsw | kCarry | kOverflow | kEndOfTable | kZero);
  }

  {
    checks.BeginCase("04 ADD mem, MDH raises ILLOPA on odd destination");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0x1234);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteInstruction(checks, executor, Mnemonic::kAdd, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFAA1),
                       std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("destination word", ReadWord(raw, 0xFAA0), 0xBEEF);
    checks.Equal("next word", ReadWord(raw, 0xFAA2), 0x1357);
    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }
}

}  // namespace c166::test::add
