#include "instructions/branch_test.h"

namespace c166::test::calls {

void TestSegmentedAbsolute(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("DA CALLS saves CSP and then IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(
        checks, executor, Mnemonic::kCallS, 4, std::nullopt,
        std::make_unique<SegmentedAddressOperand>(kFarTarget.segment, kFarTarget.offset));

    checks.Equal("return IP", ReadWord(raw, kResetStackPointer - 4), kBranchStart.offset + 4);
    checks.Equal("saved CSP", ReadWord(raw, kResetStackPointer - 2), kBranchStart.segment);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 4);
    CheckCompleted(checks, machine, kFarTarget, kInitialPsw);
  }

  {
    checks.BeginCase("DA CALLS keeps segment zero when SGTDIS is set");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSYSCON, kSegmentationDisabled);

    ExecuteInstruction(
        checks, executor, Mnemonic::kCallS, 4, std::nullopt,
        std::make_unique<SegmentedAddressOperand>(kFarTarget.segment, kFarTarget.offset));

    checks.Equal("return IP", ReadWord(raw, kResetStackPointer - 4), kStart.offset + 4);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 4);
    CheckCompleted(checks, machine, CodeAddress{0, kFarTarget.offset}, kInitialPsw);
  }

  {
    checks.BeginCase("DA CALLS raises STKOF when the second word crosses STKOV");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB22);
    machine.WriteRegister(kSTKOV, 0xFB20);

    ExecuteInstruction(
        checks, executor, Mnemonic::kCallS, 4, std::nullopt,
        std::make_unique<SegmentedAddressOperand>(kFarTarget.segment, kFarTarget.offset));

    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
  }

  {
    checks.BeginCase("DA CALLS raises ILLINA on an odd target");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kCallS, 4, std::nullopt,
                       std::make_unique<SegmentedAddressOperand>(kFarTarget.segment, kOddTarget));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::calls
