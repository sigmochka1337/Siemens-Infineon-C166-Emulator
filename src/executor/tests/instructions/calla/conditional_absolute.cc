#include "instructions/branch_test.h"

namespace c166::test::calla {

void TestConditionalAbsolute(Checks& checks) {
  using enum RegisterId;

  for (const auto& test : kBranchConditions) {
    checks.BeginCase(test.name);
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    const Word psw = kControlPsw | test.flags;
    Prepare(machine, psw, kBranchStart);
    machine.Write16(kResetStackPointer - 2, 0xA55A);

    ExecuteInstruction(checks, executor, Mnemonic::kCallA, 4, test.condition,
                       std::make_unique<CodeAddressOperand>(kNearTarget.offset));

    checks.Equal("SP", machine.ReadRegister(kSP),
                 test.taken ? kResetStackPointer - 2 : kResetStackPointer);
    checks.Equal("stack word", ReadWord(raw, kResetStackPointer - 2),
                 test.taken ? kBranchStart.offset + 4 : 0xA55A);
    const CodeAddress next =
        test.taken ? kNearTarget : CodeAddress{kBranchStart.segment, kBranchStart.offset + 4};
    CheckCompleted(checks, machine, next, psw);
  }

  {
    checks.BeginCase("CA CALLA skips an odd target when cc_Z is false");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kCallA, 4, ConditionCode::kZ,
                       std::make_unique<CodeAddressOperand>(kOddTarget));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, kBranchStart.offset + 4},
                   kPreservedPsw);
  }

  {
    checks.BeginCase("CA CALLA raises ILLINA on a taken odd target");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kCallA, 4, ConditionCode::kUC,
                       std::make_unique<CodeAddressOperand>(kOddTarget));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }

  {
    checks.BeginCase("CA CALLA raises STKOF when the return address crosses STKOV");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKOV, 0xFB20);

    ExecuteInstruction(checks, executor, Mnemonic::kCallA, 4, ConditionCode::kUC,
                       std::make_unique<CodeAddressOperand>(kNearTarget.offset));

    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
  }
}

}  // namespace c166::test::calla
