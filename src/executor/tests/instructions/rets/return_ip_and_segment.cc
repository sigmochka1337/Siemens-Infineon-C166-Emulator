#include "instructions/branch_test.h"

namespace c166::test::rets {

void TestReturnIpAndSegment(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("DB 00 RETS pops IP then CSP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kFarTarget.offset);
    machine.Write16(kResetStackPointer - 2, kFarTarget.segment);

    ExecuteInstruction(checks, executor, Mnemonic::kRetS, 2);

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, kFarTarget, kInitialPsw);
  }

  {
    checks.BeginCase("DB 00 RETS keeps segment zero when SGTDIS is set");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSYSCON, kSegmentationDisabled);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kFarTarget.offset);
    machine.Write16(kResetStackPointer - 2, kFarTarget.segment);

    ExecuteInstruction(checks, executor, Mnemonic::kRetS, 2);

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{0, kFarTarget.offset}, kInitialPsw);
  }

  {
    checks.BeginCase("DB 00 RETS raises STKUF when the second pop crosses STKUN");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB1E);
    machine.WriteRegister(kSTKUN, 0xFB20);
    machine.Write16(0xFB1E, kFarTarget.offset);
    machine.Write16(0xFB20, kFarTarget.segment);

    ExecuteInstruction(checks, executor, Mnemonic::kRetS, 2);

    CheckHardwareTrap(checks, machine, kStackUnderflowVector, kStackUnderflow);
  }

  {
    checks.BeginCase("DB 00 RETS raises ILLINA on an odd return IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kOddTarget);
    machine.Write16(kResetStackPointer - 2, kFarTarget.segment);

    ExecuteInstruction(checks, executor, Mnemonic::kRetS, 2);

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::rets
