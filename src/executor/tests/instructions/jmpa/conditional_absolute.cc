#include "instructions/branch_test.h"

namespace c166::test::jmpa {

void TestConditionalAbsolute(Checks& checks) {
  using enum RegisterId;

  for (const auto& test : kBranchConditions) {
    checks.BeginCase(test.name);
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    const Word psw = kControlPsw | test.flags;
    Prepare(machine, psw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpA, 4, test.condition,
                       std::make_unique<CodeAddressOperand>(kNearTarget.offset));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    const CodeAddress next =
        test.taken ? kNearTarget : CodeAddress{kBranchStart.segment, kBranchStart.offset + 4};
    CheckCompleted(checks, machine, next, psw);
  }

  {
    checks.BeginCase("EA JMPA skips an odd target when cc_Z is false");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpA, 4, ConditionCode::kZ,
                       std::make_unique<CodeAddressOperand>(kOddTarget));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, kBranchStart.offset + 4},
                   kPreservedPsw);
  }

  {
    checks.BeginCase("EA JMPA raises ILLINA on a taken odd target");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpA, 4, ConditionCode::kUC,
                       std::make_unique<CodeAddressOperand>(kOddTarget));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::jmpa
