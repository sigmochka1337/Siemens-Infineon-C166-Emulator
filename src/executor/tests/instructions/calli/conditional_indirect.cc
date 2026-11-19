#include "instructions/branch_test.h"

namespace c166::test::calli {

void TestConditionalIndirect(Checks& checks) {
  using enum RegisterId;

  for (const auto& test : kBranchConditions) {
    checks.BeginCase(test.name);
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    const Word psw = kControlPsw | test.flags;
    Prepare(machine, psw, kBranchStart);
    machine.WriteRegister(kR15, kNearTarget.offset);
    // The GPR contains the target IP; data at that address has a different value.
    machine.Write16(kNearTarget.offset, 0x0ABC);
    machine.Write16(kResetStackPointer - 2, 0xA55A);

    ExecuteInstruction(checks, executor, Mnemonic::kCallI, 2, test.condition,
                       IndirectMemoryOperand::CreatePlain(kR15));

    checks.Equal("R15", machine.ReadRegister(kR15), kNearTarget.offset);
    checks.Equal("SP", machine.ReadRegister(kSP),
                 test.taken ? kResetStackPointer - 2 : kResetStackPointer);
    checks.Equal("stack word", ReadWord(raw, kResetStackPointer - 2),
                 test.taken ? kBranchStart.offset + 2 : 0xA55A);
    const CodeAddress next =
        test.taken ? kNearTarget : CodeAddress{kBranchStart.segment, kBranchStart.offset + 2};
    CheckCompleted(checks, machine, next, psw);
  }

  {
    checks.BeginCase("AB CALLI skips an odd target when cc_Z is false");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);
    machine.WriteRegister(kR15, kOddTarget);

    ExecuteInstruction(checks, executor, Mnemonic::kCallI, 2, ConditionCode::kZ,
                       IndirectMemoryOperand::CreatePlain(kR15));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, kBranchStart.offset + 2},
                   kPreservedPsw);
  }

  {
    checks.BeginCase("AB CALLI raises ILLINA on a taken odd target");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kR15, kOddTarget);

    ExecuteInstruction(checks, executor, Mnemonic::kCallI, 2, ConditionCode::kUC,
                       IndirectMemoryOperand::CreatePlain(kR15));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }

  {
    checks.BeginCase("AB CALLI raises STKOF when the return address crosses STKOV");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kR15, kNearTarget.offset);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKOV, 0xFB20);

    ExecuteInstruction(checks, executor, Mnemonic::kCallI, 2, ConditionCode::kUC,
                       IndirectMemoryOperand::CreatePlain(kR15));

    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
  }
}

}  // namespace c166::test::calli
