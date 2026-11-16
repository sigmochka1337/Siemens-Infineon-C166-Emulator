#include "instructions/branch_test.h"

namespace c166::test::jmpi {

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

    ExecuteInstruction(checks, executor, Mnemonic::kJmpI, 2, test.condition,
                       IndirectMemoryOperand::CreatePlain(kR15));

    checks.Equal("R15", machine.ReadRegister(kR15), kNearTarget.offset);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    const CodeAddress next =
        test.taken ? kNearTarget : CodeAddress{kBranchStart.segment, kBranchStart.offset + 2};
    CheckCompleted(checks, machine, next, psw);
  }

  {
    checks.BeginCase("9C JMPI skips an odd target when cc_Z is false");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);
    machine.WriteRegister(kR15, kOddTarget);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpI, 2, ConditionCode::kZ,
                       IndirectMemoryOperand::CreatePlain(kR15));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, kBranchStart.offset + 2},
                   kPreservedPsw);
  }

  {
    checks.BeginCase("9C JMPI raises ILLINA on a taken odd target");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kR15, kOddTarget);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpI, 2, ConditionCode::kUC,
                       IndirectMemoryOperand::CreatePlain(kR15));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::jmpi
