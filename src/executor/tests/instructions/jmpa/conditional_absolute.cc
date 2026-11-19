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

  {
    checks.BeginCase("EA JMPA defers ILLINA inside a class A handler");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKOV, 0xFB20);

    ExecuteInstruction(checks, executor, Mnemonic::kPush, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0));
    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
    const Word class_a_sp = machine.ReadRegister(kSP);

    ExecuteInstruction(checks, executor, Mnemonic::kJmpA, 4, ConditionCode::kUC,
                       std::make_unique<CodeAddressOperand>(kOddTarget));

    checks.Equal("TFR", machine.ReadRegister(kTFR), kStackOverflow | kIllegalInstruction);
    checks.Equal("SP inside class A handler", machine.ReadRegister(kSP), class_a_sp);
    checks.Equal("IP bit 0", machine.GetPC().offset & 1u, 0);

    // After class A returns, the pending class B trap uses the resumed IP (p. 3-31).
    machine.WriteRegister(kSTKOV, 0xFA00);
    machine.WriteRegister(kTFR, kIllegalInstruction);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);

    CheckHardwareTrap(checks, machine, kClassBVector, kIllegalInstruction);
    checks.Equal("saved IP", ReadWord(raw, machine.ReadRegister(kSP)), kBranchStart.offset + 2);
  }
}

}  // namespace c166::test::jmpa
