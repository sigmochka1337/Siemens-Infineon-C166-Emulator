#include "instructions/instruction_test.h"

namespace c166::test::xor_word {

void TestImmediate16ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("XOR MDH, #data16");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kMDH, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0x8000));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x4531);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable);
  }

  {
    checks.BeginCase("XOR explicit PSW write overrides the implicit flags");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(0x0043));
    CheckCompleted(checks, machine, 4, 0x201C);
  }

  {
    checks.BeginCase("XOR decoded short SFR address bypasses DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 0);
    machine.WriteRegister(kMDH, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kXor, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFE0C),
                       std::make_unique<ImmediateOperand>(0x0033));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xC502);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }
}

}  // namespace c166::test::xor_word
