#include "instructions/branch_test.h"

namespace c166::test::jmps {

void TestSegmentedAbsolute(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("FA JMPS changes CSP and IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(
        checks, executor, Mnemonic::kJmpS, 4, std::nullopt,
        std::make_unique<SegmentedAddressOperand>(kFarTarget.segment, kFarTarget.offset));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, kFarTarget, kInitialPsw);
  }

  {
    checks.BeginCase("FA JMPS keeps segment zero when SGTDIS is set");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSYSCON, kSegmentationDisabled);

    ExecuteInstruction(
        checks, executor, Mnemonic::kJmpS, 4, std::nullopt,
        std::make_unique<SegmentedAddressOperand>(kFarTarget.segment, kFarTarget.offset));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{0, kFarTarget.offset}, kInitialPsw);
  }

  {
    checks.BeginCase("FA JMPS raises ILLINA on an odd target");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpS, 4, std::nullopt,
                       std::make_unique<SegmentedAddressOperand>(kFarTarget.segment, kOddTarget));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::jmps
