#include "instructions/instruction_test.h"

namespace c166::test::trap {

void TestTrapNumber(Checks& checks) {
  using enum RegisterId;

  for (bool segmented : {true, false}) {
    checks.BeginCase("TRAP #127 saves the next IP and preserves interrupt priority");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, {static_cast<Byte>(segmented ? 3 : 0), 0x400});
    if (!segmented) machine.WriteRegister(kSYSCON, kSegmentationDisabled);
    ExecuteInstruction(checks, executor, Mnemonic::kTrap, 2, std::nullopt,
                       std::make_unique<ImmediateOperand>(127));
    CheckCompleted(checks, machine, {0, 0x01FC}, kInitialPsw);
    const Word frame = kResetStackPointer - (segmented ? 6 : 4);
    checks.Equal("SP", machine.ReadRegister(kSP), frame);
    checks.Equal("saved IP", ReadWord(raw, frame), 0x402);
    if (segmented) checks.Equal("saved CSP", ReadWord(raw, frame + 2), 3);
    checks.Equal("saved PSW", ReadWord(raw, kResetStackPointer - 2), kInitialPsw);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);
    CheckCompleted(checks, machine, {static_cast<Byte>(segmented ? 3 : 0), 0x402}, kInitialPsw);
    checks.Equal("restored SP", machine.ReadRegister(kSP), kResetStackPointer);
  }

  {
    checks.BeginCase("A software TRAP inside a class B handler returns to the handler first");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kTFR, kIllegalWord);
    ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
    ExecuteInstruction(checks, executor, Mnemonic::kTrap, 2, std::nullopt,
                       std::make_unique<ImmediateOperand>(64));
    CheckPc(checks, machine, {0, 0x0100});
    checks.Equal("nested SP", machine.ReadRegister(kSP), kResetStackPointer - 12);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);
    CheckPc(checks, machine, {0, 0x002A});
    checks.Equal("hardware frame remains", machine.ReadRegister(kSP), kResetStackPointer - 6);
    machine.WriteRegister(kTFR, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kReti, 2);
    CheckCompleted(checks, machine, 2, kInitialPsw);
  }

  checks.BeginCase("TRAP rejects a vector outside seven bits");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  CheckInvalidInstruction(checks, executor, Mnemonic::kTrap, 2,
                          std::make_unique<ImmediateOperand>(128));
  CheckCompleted(checks, machine, kStart, kInitialPsw);
}

}  // namespace c166::test::trap
