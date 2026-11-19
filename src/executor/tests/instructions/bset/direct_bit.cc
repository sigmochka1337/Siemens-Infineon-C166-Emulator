#include "instructions/instruction_test.h"

namespace c166::test::bset {

void TestDirectBit(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("BSET 0xFD00.0 ignores DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 5);
    machine.Write16(0xFD00, 0x3456);
    ExecuteInstruction(checks, executor, Mnemonic::kBSet, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFD00, 0));
    checks.Equal("bit-addressed word", ReadWord(raw, 0xFD00), 0x3457);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  {
    checks.BeginCase("BSET 0xFDFE.15 ignores DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 5);
    machine.Write16(0xFDFE, 0xA345);
    ExecuteInstruction(checks, executor, Mnemonic::kBSet, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFDFE, 15));
    checks.Equal("bit-addressed word", ReadWord(raw, 0xFDFE), 0xA345);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("BSET R15.7 follows CP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kCP, 0xFA00);
    machine.WriteRegister(kR15, 0x3480);
    machine.Write16(0xFFFE, 0x6E6E);
    ExecuteInstruction(checks, executor, Mnemonic::kBSet, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFFFE, 7));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x3480);
    checks.Equal("encoded address", ReadWord(raw, 0xFFFE), 0x6E6E);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("BSET PSW.6 preserves the other PSW bits");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, 0x201F);
    ExecuteInstruction(checks, executor, Mnemonic::kBSet, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFF10, 6));
    CheckCompleted(checks, machine, 2, 0x205F);
  }

  {
    checks.BeginCase("BSET rejects bit position 16");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kBSet, 2,
                            std::make_unique<BitOperand>(0xFD00, 16));
  }

  {
    checks.BeginCase("BSET rejects non-bit-addressable memory");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kBSet, 2,
                            std::make_unique<BitOperand>(0x0200, 0));
  }

  {
    checks.BeginCase("BSET resolved GPR word outside the bit-addressable DPRAM window");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kCP, 0xFA00);
    machine.WriteRegister(kR10, 0x5000);

    ExecuteInstruction(checks, executor, Mnemonic::kBSet, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFA14, 0));

    checks.Equal("R10", machine.ReadRegister(kR10), 0x5001);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero);
  }

  for (const Byte bit : {0, 1, 2, 3, 7, 13, 14, 15}) {
    checks.BeginCase("BSET of a TFR request flag enters its hardware trap");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kBSet, 2, std::nullopt,
                       std::make_unique<BitOperand>(0xFFAC, bit));
    const CodeAddress vector = bit == 15   ? CodeAddress{0, 8}
                               : bit == 14 ? kStackOverflowVector
                               : bit == 13 ? kStackUnderflowVector
                                           : kClassBVector;
    CheckHardwareTrap(checks, machine, vector, 1u << bit);
    checks.Equal("saved IP", ReadWord(raw, kResetStackPointer - 6), kStart.offset + 2);
  }
}

}  // namespace c166::test::bset
