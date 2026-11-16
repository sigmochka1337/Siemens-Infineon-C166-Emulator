#include "instructions/branch_test.h"

namespace c166::test::ret {

void TestReturnIp(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("CB 00 RET restores the IP in the current segment");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 2);
    machine.Write16(kResetStackPointer - 2, kNearTarget.offset);

    ExecuteInstruction(checks, executor, Mnemonic::kRet, 2);

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, kNearTarget, kInitialPsw);
  }

  {
    checks.BeginCase("CB 00 RET raises STKUF when its pop crosses STKUN");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKUN, 0xFB20);
    machine.Write16(0xFB20, kNearTarget.offset);

    ExecuteInstruction(checks, executor, Mnemonic::kRet, 2);

    CheckHardwareTrap(checks, machine, kStackUnderflowVector, kStackUnderflow);
  }

  {
    checks.BeginCase("CB 00 RET raises ILLINA on an odd return IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 2);
    machine.Write16(kResetStackPointer - 2, kOddTarget);

    ExecuteInstruction(checks, executor, Mnemonic::kRet, 2);

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::ret
