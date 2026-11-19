#include "instructions/branch_test.h"

namespace c166::test::push {

void TestRegisterToStack(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("EC PUSH R12 writes a word after decrementing SP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR12, 0);
    machine.WriteRegister(kDPP3, 0);
    machine.Write16(kResetStackPointer - 2, 0xA55A);
    machine.Write16(kResetStackPointer - 4, 0x1234);

    ExecuteInstruction(checks, executor, Mnemonic::kPush, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12));

    checks.Equal("pushed R12", ReadWord(raw, kResetStackPointer - 2), 0);
    checks.Equal("adjacent word", ReadWord(raw, kResetStackPointer - 4), 0x1234);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 2);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kZero);
  }

  {
    checks.BeginCase("EC PUSH MDH with a minimum signed word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw);
    machine.WriteRegister(kMDH, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kPush, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("pushed MDH", ReadWord(raw, kResetStackPointer - 2), 0x8000);
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x8000);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 2);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("EC PUSH SP pushes the original SP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);

    ExecuteInstruction(checks, executor, Mnemonic::kPush, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kSP));

    checks.Equal("pushed SP", ReadWord(raw, kResetStackPointer - 2), kResetStackPointer);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 2);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("EC PUSH wraps the physical address of a 64-word circular stack");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    // STKSZ=010 maps the stack to FB80H–FBFEH (table 3-15).
    machine.WriteRegister(kSYSCON, 0x4000);
    machine.WriteRegister(kSP, 0xFB80);
    machine.WriteRegister(kR12, 0x1234);
    machine.Write16(0xFB7E, 0xA55A);

    ExecuteInstruction(checks, executor, Mnemonic::kPush, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12));

    checks.Equal("physical stack word", ReadWord(raw, 0xFBFE), 0x1234);
    checks.Equal("word below stack area", ReadWord(raw, 0xFB7E), 0xA55A);
    checks.Equal("SP", machine.ReadRegister(kSP), 0xFB7E);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }

  {
    checks.BeginCase("EC PUSH saves the next IP when the stack overflows");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKOV, 0xFB20);
    machine.WriteRegister(kMDH, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kPush, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));

    CheckHardwareTrap(checks, machine, kStackOverflowVector, kStackOverflow);
    checks.Equal("SP", machine.ReadRegister(kSP), 0xFB18);
    checks.Equal("saved IP", ReadWord(raw, 0xFB18), kBranchStart.offset + 2);
    checks.Equal("saved CSP", ReadWord(raw, 0xFB1A), kBranchStart.segment);
    checks.Equal("saved PSW", ReadWord(raw, 0xFB1C), kPreservedPsw | kNegative | kEndOfTable);
    checks.Equal("pushed MDH", ReadWord(raw, 0xFB1E), 0x8000);
  }
}

}  // namespace c166::test::push
