#include "instructions/instruction_test.h"

namespace c166::test::bclr {

void TestDirectBit(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("BCLR 0xFD00.0 ignores DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 5);
    machine.Write16(0xFD00, 0x3456);
    ExecuteInstruction(checks, executor, Mnemonic::kBClr, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFD00, 0));
    checks.Equal("bit-addressed word", ReadWord(raw, 0xFD00), 0x3456);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("BCLR 0xFDFE.15 ignores DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 5);
    machine.Write16(0xFDFE, 0xA345);
    ExecuteInstruction(checks, executor, Mnemonic::kBClr, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFDFE, 15));
    checks.Equal("bit-addressed word", ReadWord(raw, 0xFDFE), 0x2345);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("BCLR R15.7 follows CP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kCP, 0xFA00);
    machine.WriteRegister(kR15, 0x3480);
    machine.Write16(0xFFFE, 0x6E6E);
    ExecuteInstruction(checks, executor, Mnemonic::kBClr, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFFFE, 7));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x3400);
    checks.Equal("encoded address", ReadWord(raw, 0xFFFE), 0x6E6E);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("BCLR PSW.6 preserves the other PSW bits");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kBClr, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFF10, 6));
    CheckCompleted(checks, machine, 2, 0x201F);
  }

  {
    checks.BeginCase("BCLR rejects bit position 16");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kBClr, 2,
                            std::make_unique<BitOperand>(0xFD00, 16));
  }

  {
    checks.BeginCase("BCLR rejects non-bit-addressable memory");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kBClr, 2,
                            std::make_unique<BitOperand>(0x0200, 0));
  }

  {
    checks.BeginCase("BCLR decoded extended SFR word bypasses DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 5);
    machine.Write16(0xF1DE, 0x8181);

    ExecuteInstruction(checks, executor, Mnemonic::kBClr, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xF1DE, 15));

    checks.Equal("bit-addressed word", ReadWord(raw, 0xF1DE), 0x0181);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }
}

}  // namespace c166::test::bclr
