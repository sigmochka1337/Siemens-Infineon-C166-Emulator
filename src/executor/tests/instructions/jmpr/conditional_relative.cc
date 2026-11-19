#include "instructions/branch_test.h"

namespace c166::test::jmpr {

void TestConditionalRelative(Checks& checks) {
  using enum RegisterId;

  for (const auto& test : kBranchConditions) {
    checks.BeginCase(test.name);
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    const Word psw = kControlPsw | test.flags;
    Prepare(machine, psw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpR, 2, test.condition,
                       std::make_unique<RelativeOperand>(15));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    const CodeAddress next = test.taken
                                 ? CodeAddress{kBranchStart.segment, 0x0220}
                                 : CodeAddress{kBranchStart.segment, kBranchStart.offset + 2};
    CheckCompleted(checks, machine, next, psw);
  }

  {
    checks.BeginCase("cD JMPR cc_UC, -128");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpR, 2, ConditionCode::kUC,
                       std::make_unique<RelativeOperand>(-128));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, 0x0102}, kInitialPsw);
  }

  {
    checks.BeginCase("cD JMPR cc_UC, +127");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpR, 2, ConditionCode::kUC,
                       std::make_unique<RelativeOperand>(127));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, 0x0300}, kInitialPsw);
  }

  {
    checks.BeginCase("cD JMPR stays in its own segment when IP wraps");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, CodeAddress{kBranchStart.segment, 0xFFFC});

    ExecuteInstruction(checks, executor, Mnemonic::kJmpR, 2, ConditionCode::kUC,
                       std::make_unique<RelativeOperand>(1));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, 0}, kInitialPsw);
  }
}

}  // namespace c166::test::jmpr
