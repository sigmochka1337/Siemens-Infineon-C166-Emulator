#include "instructions/branch_test.h"

namespace c166::test::retp {

void TestReturnIpAndRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("EB RETP R12 pops IP before the register word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kNearTarget.offset);
    machine.Write16(kResetStackPointer - 2, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kRetP, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12));

    checks.Equal("R12", machine.ReadRegister(kR12), 0);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, kNearTarget, kPreservedPsw | kZero);
  }

  {
    checks.BeginCase("EB RETP MDH sets flags from the register word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kNearTarget.offset);
    machine.Write16(kResetStackPointer - 2, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kRetP, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x8000);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, kNearTarget, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("EB RETP PSW restores the popped word to PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kNearTarget.offset);
    machine.Write16(kResetStackPointer - 2, 0x104A);

    ExecuteInstruction(checks, executor, Mnemonic::kRetP, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, kNearTarget, 0x104A);
  }

  {
    checks.BeginCase("EB RETP raises STKUF when the second pop crosses STKUN");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB1E);
    machine.WriteRegister(kSTKUN, 0xFB20);
    machine.Write16(0xFB1E, kNearTarget.offset);
    machine.Write16(0xFB20, 0x1234);

    ExecuteInstruction(checks, executor, Mnemonic::kRetP, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));

    CheckHardwareTrap(checks, machine, kStackUnderflowVector, kStackUnderflow);
  }

  {
    checks.BeginCase("EB RETP raises ILLINA on an odd return IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, kBranchStart);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kOddTarget);
    machine.Write16(kResetStackPointer - 2, 0x1234);

    ExecuteInstruction(checks, executor, Mnemonic::kRetP, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }
}

}  // namespace c166::test::retp
