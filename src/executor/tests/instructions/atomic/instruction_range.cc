#include "instructions/instruction_test.h"

namespace c166::test::atomic {

void TestInstructionRange(Checks& checks) {
  using enum RegisterId;

  for (Word count = 1; count <= 4; ++count) {
    checks.BeginCase("ATOMIC defers NMI until the last instruction");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kAtomic, 2, std::nullopt,
                       std::make_unique<ImmediateOperand>(count));
    machine.WriteRegister(kTFR, 0x8000);
    for (Word step = 1; step < count; ++step) {
      ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
      CheckPc(checks, machine, {0, static_cast<Word>(kStart.offset + 2 + step * 2)});
    }
    ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
    CheckHardwareTrap(checks, machine, {0, 8}, 0x8000);
    checks.Equal("saved IP", ReadWord(raw, kResetStackPointer - 6), kStart.offset + 2 + 2 * count);
  }

  {
    checks.BeginCase("A nested ATOMIC reloads the instruction count");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kAtomic, 2, std::nullopt,
                       std::make_unique<ImmediateOperand>(4));
    ExecuteInstruction(checks, executor, Mnemonic::kAtomic, 2, std::nullopt,
                       std::make_unique<ImmediateOperand>(1));
    machine.WriteRegister(kTFR, 0x8000);
    ExecuteInstruction(checks, executor, Mnemonic::kNop, 2);
    CheckHardwareTrap(checks, machine, {0, 8}, 0x8000);
  }
}

}  // namespace c166::test::atomic
