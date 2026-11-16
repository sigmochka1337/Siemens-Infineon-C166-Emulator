#include "instructions/branch_test.h"

namespace c166::test::pop {

void TestStackToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("FC POP R12 reads the old SP and increments it by two");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSP, kResetStackPointer - 4);
    machine.WriteRegister(kR12, 0x2468);
    machine.WriteRegister(kDPP3, 0);
    machine.Write16(kResetStackPointer - 4, 0);
    machine.Write16(kResetStackPointer - 2, 0xA55A);

    ExecuteInstruction(checks, executor, Mnemonic::kPop, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12));

    checks.Equal("R12", machine.ReadRegister(kR12), 0);
    checks.Equal("popped stack word", ReadWord(raw, kResetStackPointer - 4), 0);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer - 2);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kZero);
  }

  {
    checks.BeginCase("FC POP MDH with a minimum signed word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw);
    machine.WriteRegister(kSP, kResetStackPointer - 2);
    machine.Write16(kResetStackPointer - 2, 0x8000);

    ExecuteInstruction(checks, executor, Mnemonic::kPop, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x8000);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("FC POP SP writes the popped value after the increment");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSP, kResetStackPointer - 2);
    machine.Write16(kResetStackPointer - 2, 0xFB40);

    ExecuteInstruction(checks, executor, Mnemonic::kPop, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kSP));

    checks.Equal("SP", machine.ReadRegister(kSP), 0xFB40);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("FC POP PSW replaces the flags with the popped word");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSP, kResetStackPointer - 2);
    machine.Write16(kResetStackPointer - 2, 0x104A);

    ExecuteInstruction(checks, executor, Mnemonic::kPop, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW));

    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, 2, 0x104A);
  }

  {
    checks.BeginCase("FC POP reads the wrapped physical address of a 64-word circular stack");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    // STKSZ=010 maps the stack to FB80H–FBFEH (table 3-15).
    machine.WriteRegister(kSYSCON, 0x4000);
    machine.WriteRegister(kSP, 0xFB7E);
    machine.Write16(0xFB7E, 0xA55A);
    machine.Write16(0xFBFE, 0x1234);

    ExecuteInstruction(checks, executor, Mnemonic::kPop, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR12));

    checks.Equal("R12", machine.ReadRegister(kR12), 0x1234);
    checks.Equal("SP", machine.ReadRegister(kSP), 0xFB80);
    CheckCompleted(checks, machine, 2, kPreservedPsw);
  }

  {
    checks.BeginCase("FC POP saves the next IP when the stack underflows");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw, kBranchStart);
    machine.WriteRegister(kSP, 0xFB20);
    machine.WriteRegister(kSTKUN, 0xFB20);
    machine.Write16(0xFB20, 0);

    ExecuteInstruction(checks, executor, Mnemonic::kPop, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH));

    CheckHardwareTrap(checks, machine, kStackUnderflowVector, kStackUnderflow);
    checks.Equal("SP", machine.ReadRegister(kSP), 0xFB1C);
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0);
    checks.Equal("saved IP", ReadWord(raw, 0xFB1C), kBranchStart.offset + 2);
    checks.Equal("saved CSP", ReadWord(raw, 0xFB1E), kBranchStart.segment);
    checks.Equal("saved PSW", ReadWord(raw, 0xFB20), kPreservedPsw | kZero);
  }
}

}  // namespace c166::test::pop
