#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestRegisterToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("F6 MOV mem, MDH");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xCC55);

    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0xFAE0),
               std::make_unique<RegisterOperand>(kMDH));

    checks.Equal("destination", ReadWord(raw, 0xFAE0), 0xCC55);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("F6 MOV mem, R3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR3, 0x1234);

    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0xFAE0),
               std::make_unique<RegisterOperand>(kR3));

    checks.Equal("destination", ReadWord(raw, 0xFAE0), 0x1234);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }

  {
    checks.BeginCase("F6 ILLOPA with segmentation disabled");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSYSCON, kSegmentationDisabled);
    machine.WriteRegister(kR3, 0x1234);
    machine.Write16(0xFAA0, 0xBEEF);
    machine.Write16(0xFAA2, 0x1357);

    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0xFAA1),
               std::make_unique<RegisterOperand>(kR3));

    checks.Equal("destination", ReadWord(raw, 0xFAA0), 0xBEEF);
    checks.Equal("adjacent word", ReadWord(raw, 0xFAA2), 0x1357);
    CheckTrap(checks, machine, raw, 4, kIllegalWord, false);
  }

  {
    checks.BeginCase("F6 MOV mem, R3 mapped above 64 KiB");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 0x40);
    machine.WriteRegister(kR3, 0x1234);
    machine.Write16(0x100000, 0xBEEF);
    machine.Write16(0x100002, 0x1357);

    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0x4000),
               std::make_unique<RegisterOperand>(kR3));

    checks.Equal("destination", ReadWord(raw, 0x100000), 0x1234);
    checks.Equal("adjacent word", ReadWord(raw, 0x100002), 0x1357);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }
}

}  // namespace c166::test::mov
