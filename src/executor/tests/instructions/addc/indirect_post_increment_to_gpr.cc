#include "instructions/instruction_test.h"

namespace c166::test::addc {

void TestIndirectPostIncrementToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("18 ADDC R0, [R3+]");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xFFFE);
    machine.WriteRegister(kR3, 0xFA80);
    machine.Write16(0xFA80, 0x0001);
    machine.Write16(0xFA82, 0x375A);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       IndirectMemoryOperand::CreatePostIncrement(kR3));

    checks.Equal("R0", machine.ReadRegister(kR0), 0);
    checks.Equal("R3", machine.ReadRegister(kR3), 0xFA82);
    CheckCompleted(checks, machine, 2, kControlPsw | kCarry | kZero);
  }

  {
    checks.BeginCase("18 ADDC R0, [R3+] raises ILLOPA on odd source");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0x1234);
    machine.WriteRegister(kR3, 0xFA81);

    ExecuteInstruction(checks, executor, Mnemonic::kAddC, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR0),
                       IndirectMemoryOperand::CreatePostIncrement(kR3));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x1234);
    CheckTrap(checks, machine, raw, 2, kIllegalWord);
  }
}

}  // namespace c166::test::addc
