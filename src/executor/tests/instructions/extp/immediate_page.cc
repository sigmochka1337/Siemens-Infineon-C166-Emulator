#include "instructions/instruction_test.h"

namespace c166::test::extp {

void TestImmediatePage(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("EXTP immediate selects data addresses for two instructions");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.Write16(0xC020, 0x3456);
    machine.Write16(0x414020, 0x1234);
    ExecuteInstruction(checks, executor, Mnemonic::kExtp, 4, std::nullopt,
                       std::make_unique<ImmediateOperand>(0x0105),
                       std::make_unique<ImmediateOperand>(2));
    CheckCompleted(checks, machine, 4, kInitialPsw);
    ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0xC020));
    checks.Equal("short SFR destination", machine.Read16(0xFE0C), 0x1234);
    machine.WriteRegister(kR3, 0xC020);
    ExecuteInstruction(checks, executor, Mnemonic::kMov, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR4),
                       IndirectMemoryOperand::CreatePlain(kR3));
    checks.Equal("indirect load", machine.ReadRegister(kR4), 0x1234);
    ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0xC020));
    checks.Equal("standard SFR and page restored", machine.ReadRegister(kMDH), 0x3456);
    CheckCompleted(checks, machine, 14, kPreservedPsw);
  }

  {
    checks.BeginCase("A class B request breaks EXTP and lets a pending NMI run first");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kExtp, 4, std::nullopt,
                       std::make_unique<ImmediateOperand>(7),
                       std::make_unique<ImmediateOperand>(4));
    machine.WriteRegister(kTFR, 0x8004);
    ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
    CheckHardwareTrap(checks, machine, {0, 8}, 0x8004);
    checks.Equal("saved IP", ReadWord(raw, kResetStackPointer - 6), kStart.offset + 6);
    machine.WriteRegister(kTFR, kIllegalWord);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);
    CheckHardwareTrap(checks, machine, kClassBVector, kIllegalWord);
    machine.WriteRegister(kTFR, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);
    machine.Write16(0x4000, 0x1234);
    ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       std::make_unique<DirectMemoryOperand>(0x4000));
    checks.Equal("standard addressing restored", machine.ReadRegister(kR0), 0x1234);
  }
}

}  // namespace c166::test::extp
