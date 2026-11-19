#include "instructions/branch_test.h"

namespace c166::test::callr {

void TestRelativeTarget(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("BB CALLR -128 saves the next IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.Write16(kResetStackPointer - 2, 0xA55A);

    ExecuteInstruction(checks, executor, Mnemonic::kCallR, 2, std::nullopt,
                       std::make_unique<RelativeOperand>(-128));

    checks.Equal("return IP", ReadWord(raw, kResetStackPointer - 2), kBranchStart.offset + 2);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 2);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, 0x0102}, kInitialPsw);
  }

  {
    checks.BeginCase("BB CALLR +127 saves the next IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kCallR, 2, std::nullopt,
                       std::make_unique<RelativeOperand>(127));

    checks.Equal("return IP", ReadWord(raw, kResetStackPointer - 2), kBranchStart.offset + 2);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 2);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, 0x0300}, kInitialPsw);
  }

  {
    checks.BeginCase("BB CALLR raises STKOF when the return address crosses STKOV");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKOV, 0xFB20);

    ExecuteInstruction(checks, executor, Mnemonic::kCallR, 2, std::nullopt,
                       std::make_unique<RelativeOperand>(15));

    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
  }
}

}  // namespace c166::test::callr
