#include "instructions/branch_test.h"

namespace c166::test::reti {

void TestInterruptFrame(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("FB 88 RETI pops IP, CSP and PSW in segmented mode");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPriorityMask);
    machine.WriteRegister(kSP, kResetStackPointer - 6);
    machine.Write16(kResetStackPointer - 6, kFarTarget.offset);
    machine.Write16(kResetStackPointer - 4, kFarTarget.segment);
    machine.Write16(kResetStackPointer - 2, 0x1054);

    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, kFarTarget, 0x1054);
  }

  {
    checks.BeginCase("FB 88 RETI pops only IP and PSW when SGTDIS is set");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPriorityMask);
    machine.WriteRegister(kSYSCON, kSegmentationDisabled);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.Write16(kResetStackPointer - 4, kNearTarget.offset);
    machine.Write16(kResetStackPointer - 2, 0x104A);

    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{0, kNearTarget.offset}, 0x104A);
  }

  {
    checks.BeginCase("FB 88 RETI resumes after a serviced stack overflow");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKOV, 0xFB20);
    machine.WriteRegister(kMDH, 0x1234);

    ExecuteInstruction(checks, executor, Mnemonic::kPush, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));
    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
    machine.WriteRegister(kTFR, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);

    checks.Equal("SP", machine.ReadRegister(kSP), 0xFB1E);
    CheckCompleted(checks, machine, CodeAddress{kBranchStart.segment, kBranchStart.offset + 2},
                   kPreservedPsw);
  }

  {
    checks.BeginCase("FB 88 RETI raises STKUF when restoring PSW crosses STKUN");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPriorityMask);
    machine.WriteRegister(kSP, 0xFB1C);
    machine.WriteRegister(kSTKUN, 0xFB20);
    machine.Write16(0xFB1C, kFarTarget.offset);
    machine.Write16(0xFB1E, kFarTarget.segment);
    machine.Write16(0xFB20, 0x1054);

    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);

    CheckHardwareTrap(checks, machine, kStackUnderflowVector, kStackUnderflow);
  }

  {
    checks.BeginCase("FB 88 RETI raises ILLINA on an odd return IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPriorityMask);
    machine.WriteRegister(kSP, kResetStackPointer - 6);
    machine.Write16(kResetStackPointer - 6, kOddTarget);
    machine.Write16(kResetStackPointer - 4, kFarTarget.segment);
    machine.Write16(kResetStackPointer - 2, 0x1054);

    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);

    CheckIllegalTarget(checks, machine, raw, kOddTarget);
  }

  {
    checks.BeginCase("RETI releases a hardware trap after software moves the stack frame");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kTFR, kIllegalWord);
    ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
    CheckHardwareTrap(checks, machine, kClassBVector, kIllegalWord);
    machine.WriteRegister(kTFR, 0);
    for (Word i = 0; i < 6; i += 2) {
      machine.Write16(0xFB80 + i, machine.Read16(kResetStackPointer - 6 + i));
    }
    machine.WriteRegister(kSP, 0xFB80);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);
    CheckCompleted(checks, machine, 2, kInitialPsw);
    machine.WriteRegister(kTFR, kIllegalWord);
    ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
    CheckHardwareTrap(checks, machine, kClassBVector, kIllegalWord);
    checks.Equal("new frame", machine.ReadRegister(kSP), 0xFB80);
  }

  {
    checks.BeginCase("Reset releases an active hardware trap for an existing executor");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    for (int reset = 0; reset < 2; ++reset) {
      machine.Reset();
      Prepare(machine);
      ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                         std::make_unique<RegisterOperand>(kR6),
                         std::make_unique<DirectMemoryOperand>(0x6001));
      CheckTrap(checks, machine, raw, 4, kIllegalWord);
    }
  }
}

}  // namespace c166::test::reti
