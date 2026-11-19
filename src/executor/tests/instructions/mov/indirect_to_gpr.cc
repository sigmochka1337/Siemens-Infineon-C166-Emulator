#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestIndirectToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("A8 MOV R0, [R1]");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA80);
    machine.Write16(0xFA80, 0x8000);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               IndirectMemoryOperand::CreatePlain(kR1));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x8000);
    checks.Equal("R1", machine.ReadRegister(kR1), 0xFA80);
    CheckCompleted(checks, machine, 2, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("A8 ILLOPA on indirect read");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA81);
    machine.WriteRegister(kR0, 0x1234);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               IndirectMemoryOperand::CreatePlain(kR1));

    CheckTrap(checks, machine, raw, 2, kIllegalWord);
    checks.Equal("R0 after rejected read", machine.ReadRegister(kR0), 0x1234);
  }

  {
    checks.BeginCase("ILLOPA stays pending until cleared before RETI");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR1, 0xFA81);

    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR0),
               IndirectMemoryOperand::CreatePlain(kR1));

    // A handler instruction can execute with ILLOPA still set.
    ExecuteMov(executor, 2, std::make_unique<RegisterOperand>(kR2),
               std::make_unique<ImmediateOperand>(1));
    checks.Equal("handler IP", machine.GetPC().offset, kClassBVector.offset + 2);
    checks.Equal("handler SP", machine.ReadRegister(kSP), kResetStackPointer - 6);

    const Instruction reti{Mnemonic::kReti, std::nullopt, {}, 2};
    executor.Execute(reti);
    CheckTrap(checks, machine, raw, 2, kIllegalWord);

    machine.WriteRegister(kTFR, 0);
    executor.Execute(reti);
    checks.Equal("restored IP", machine.GetPC().offset, kStart.offset + 2);
    checks.Equal("restored SP", machine.ReadRegister(kSP), kResetStackPointer);
    checks.Equal("restored PSW", machine.ReadRegister(kPSW), kInitialPsw);
    checks.Equal("cleared TFR", machine.ReadRegister(kTFR), 0);
  }
}

}  // namespace c166::test::mov
