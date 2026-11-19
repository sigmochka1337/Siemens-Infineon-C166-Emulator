#include "instructions/instruction_test.h"

namespace c166::test::or_word {

void TestImmediate16ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("OR MDH, #data16");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kMDH, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0x8000));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xC531);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kNegative);
  }

  {
    checks.BeginCase("OR explicit PSW write overrides the implicit flags");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(0x0043));
    CheckCompleted(checks, machine, 4, 0x205F);
  }

  {
    checks.BeginCase("OR decoded short SFR address bypasses DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 0);
    machine.WriteRegister(kMDH, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFE0C),
                       std::make_unique<ImmediateOperand>(0x0033));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xC533);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }
}

}  // namespace c166::test::or_word
