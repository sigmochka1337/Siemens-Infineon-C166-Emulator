#include "instructions/instruction_test.h"

namespace c166::test::orb {

void TestImmediate8ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("ORB MDH, #data8");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kControlPsw);
    machine.WriteRegister(kMDH, 0x73C5);
    ExecuteInstruction(checks, executor, Mnemonic::kOrB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<ImmediateOperand>(0x0080));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x00C5);
    CheckCompleted(checks, machine, 4, kControlPsw | kEndOfTable | kNegative);
  }

  {
    checks.BeginCase("ORB explicit PSW write overrides the implicit flags");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kOrB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kPSW),
                       std::make_unique<ImmediateOperand>(0x0043));
    CheckCompleted(checks, machine, 4, 0x005F);
  }

  {
    checks.BeginCase("ORB decoded short SFR address bypasses DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 0);
    machine.WriteRegister(kMDH, 0x00C5);
    ExecuteInstruction(checks, executor, Mnemonic::kOrB, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0xFE0C),
                       std::make_unique<ImmediateOperand>(0x0033));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x00F7);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("ORB rejects a ninth immediate bit");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kOrB, 4,
                            std::make_unique<RegisterOperand>(kRH7),
                            std::make_unique<ImmediateOperand>(0x0100));
  }
}

}  // namespace c166::test::orb
