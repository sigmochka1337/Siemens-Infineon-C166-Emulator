#include "instructions/mov/mov_test.h"

namespace c166::test::mov {

void TestImmediate16ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("E6 MOV MDH, #0xCAFE");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kMDH),
               std::make_unique<ImmediateOperand>(0xCAFE));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xCAFE);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("E6 MOV R0, #0xCAFE");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kR0),
               std::make_unique<ImmediateOperand>(0xCAFE));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xCAFE);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("E6 explicit write to PSW (p. 5-25)");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kPSW),
               std::make_unique<ImmediateOperand>(kOverflow));

    CheckCompleted(checks, machine, 4, kOverflow);
  }

  {
    checks.BeginCase("E6 MOV SP below STKOV (p. 3-64)");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSTKOV, 0xFB00);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kSP),
               std::make_unique<ImmediateOperand>(0xFAC0));

    checks.Equal("SP", machine.ReadRegister(kSP), 0xFAC0);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("E6 MOV to fixed SP bits (p. 3-61)");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);

    ExecuteMov(executor, 4, std::make_unique<RegisterOperand>(kSP),
               std::make_unique<ImmediateOperand>(0x0AC1));

    checks.Equal("SP", machine.ReadRegister(kSP), 0xFAC0);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }
  {
    checks.BeginCase("MOV SP in the legacy short SFR representation bypasses DPP3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP3, 0);
    ExecuteMov(executor, 4, std::make_unique<DirectMemoryOperand>(0xFE12),
               std::make_unique<ImmediateOperand>(0xFB00));
    checks.Equal("SP", machine.ReadRegister(kSP), 0xFB00);
    checks.Equal("paged address", machine.Read16(0x3E12), 0);
  }
}

}  // namespace c166::test::mov
