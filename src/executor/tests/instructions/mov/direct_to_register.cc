#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("F2 MOV MDH, mem");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.Write16(0xFAC0, 0x2468);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kMDH),
               std::make_unique<DirectMemoryOperand>(0xFAC0));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x2468);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }

  {
    checks.BeginCase("F2 MOV R0, mem");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.Write16(0xFAC0, 0x2468);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<DirectMemoryOperand>(0xFAC0));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x2468);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }

  {
    checks.BeginCase("F2 MOV R0, mem via DPP0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP0, 3);
    machine.Write16(0xFA80, 0x8000);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<DirectMemoryOperand>(0x3A80));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x8000);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("F2 ILLOPA on direct read");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<DirectMemoryOperand>(0xFAC1));

    CheckTrap(checks, machine, raw, 4, kIllegalWord);
  }

  {
    checks.BeginCase("F2 MOV R0, mem mapped above 64 KiB");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 0x40);
    machine.Write16(0x100000, 0x5678);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<DirectMemoryOperand>(0x4000));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x5678);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }

  {
    checks.BeginCase("The instruction after a DPP write still uses the old page");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.Write16(0x4000, 0x1111);
    machine.Write16(0x14000, 0x5555);
    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kDPP1),
               std::make_unique<ImmediateOperand>(5));
    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR7),
               std::make_unique<DirectMemoryOperand>(0x4000));
    checks.Equal("first load", machine.ReadRegister(kR7), 0x1111);
    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR7),
               std::make_unique<DirectMemoryOperand>(0x4000));
    checks.Equal("second load", machine.ReadRegister(kR7), 0x5555);
    CheckCompleted(checks, machine, 12, kPreservedPsw);
  }
}

}  // namespace c166::test::mov
