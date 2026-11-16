#include "instructions/branch_test.h"

namespace c166::test::pcall {

void TestRegisterAndAbsoluteTarget(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("E2 PCALL R12, caddr pushes the operand before the return IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kR12, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kPCall, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12),
                       std::make_unique<CodeAddressOperand>(kNearTarget.offset));

    checks.Equal("return IP", ReadWord(raw, kResetStackPointer - 4), kBranchStart.offset + 4);
    checks.Equal("pushed R12", ReadWord(raw, kResetStackPointer - 2), 0);
    checks.Equal("R12", machine.ReadRegister(kR12), 0);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 4);
    CheckCompleted(checks, machine, kNearTarget, kPreservedPsw | kZero);
  }

  {
    checks.BeginCase("E2 PCALL MDH, caddr sets flags from the pushed word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);
    machine.WriteRegister(kMDH, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kPCall, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<CodeAddressOperand>(kNearTarget.offset));

    checks.Equal("return IP", ReadWord(raw, kResetStackPointer - 4), kBranchStart.offset + 4);
    checks.Equal("pushed MDH", ReadWord(raw, kResetStackPointer - 2), 0x8000);
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x8000);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 4);
    CheckCompleted(checks, machine, kNearTarget, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("E2 PCALL SP, caddr pushes the original SP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kPCall, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kSP),
                       std::make_unique<CodeAddressOperand>(kNearTarget.offset));

    checks.Equal("return IP", ReadWord(raw, kResetStackPointer - 4), kBranchStart.offset + 4);
    checks.Equal("pushed SP", ReadWord(raw, kResetStackPointer - 2), kResetStackPointer);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 4);
    CheckCompleted(checks, machine, kNearTarget, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("E2 PCALL raises STKOF when the second word crosses STKOV");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB22);
    machine.WriteRegister(kSTKOV, 0xFB20);

    ExecuteInstruction(checks, executor, Mnemonic::kPCall, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12),
                       std::make_unique<CodeAddressOperand>(kNearTarget.offset));

    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
  }

  {
    checks.BeginCase("E2 PCALL raises ILLINA on an odd target");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kPCall, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12),
                       std::make_unique<CodeAddressOperand>(kOddTarget));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::pcall
