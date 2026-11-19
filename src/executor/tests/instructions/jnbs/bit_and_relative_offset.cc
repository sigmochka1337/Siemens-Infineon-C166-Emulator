#include "instructions/instruction_test.h"

namespace c166::test::jnbs {

void TestBitAndRelativeOffset(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("JNBS FD00.15, -128 falls through");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, {7, 0x0004});
    machine.WriteRegister(kDPP3, 0);
    machine.Write16(0xFD00, 0xA345);
    ExecuteInstruction(checks, executor, Mnemonic::kJnbs, 4, std::nullopt,
                       std::make_unique<BitOperand>(0xFD00, 15),
                       std::make_unique<RelativeOperand>(-128));
    checks.Equal("bit-addressed word", ReadWord(raw, 0xFD00), 0xA345);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{7, 0x0008}, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("JNBS FD00.0, +127 taken");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kInitialPsw, {7, 0xFFF8});
    machine.WriteRegister(kDPP3, 0);
    machine.Write16(0xFD00, 0x3456);
    ExecuteInstruction(checks, executor, Mnemonic::kJnbs, 4, std::nullopt,
                       std::make_unique<BitOperand>(0xFD00, 0),
                       std::make_unique<RelativeOperand>(127));
    checks.Equal("bit-addressed word", ReadWord(raw, 0xFD00), 0x3457);
    checks.Equal("SP", machine.ReadRegister(kSP), kResetStackPointer);
    CheckCompleted(checks, machine, CodeAddress{7, 0x00FA}, kControlPsw | kZero);
  }

  {
    checks.BeginCase("JNBS R0.3 uses the active register bank");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kCP, 0xF900);
    machine.WriteRegister(kR0, 0x1200);
    machine.Write16(0xFFE0, 0x1208);
    ExecuteInstruction(checks, executor, Mnemonic::kJnbs, 4, std::nullopt,
                       std::make_unique<BitOperand>(0xFFE0, 3),
                       std::make_unique<RelativeOperand>(-2));
    checks.Equal("R0", machine.ReadRegister(kR0), 0x1208);
    checks.Equal("encoded address", ReadWord(raw, 0xFFE0), 0x1208);
    CheckCompleted(checks, machine, kStart, kControlPsw | kZero);
  }

  {
    checks.BeginCase("JNBS PSW.6 tests the original PSW");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, 0x201F);
    ExecuteInstruction(checks, executor, Mnemonic::kJnbs, 4, std::nullopt,
                       std::make_unique<BitOperand>(0xFF10, 6),
                       std::make_unique<RelativeOperand>(1));
    CheckCompleted(checks, machine, CodeAddress{0, 0x0206}, 0x205F);
  }

  {
    checks.BeginCase("JNBS rejects displacement -129 before changing memory");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.Write16(0xFD00, 0x0000);
    CheckInvalidInstruction(checks, executor, Mnemonic::kJnbs, 4,
                            std::make_unique<BitOperand>(0xFD00, 0),
                            std::make_unique<RelativeOperand>(-129));
    checks.Equal("source word", ReadWord(raw, 0xFD00), 0x0000);
    checks.Equal("PSW", machine.ReadRegister(kPSW), kInitialPsw);
  }

  {
    checks.BeginCase("JNBS rejects displacement +128 before changing memory");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.Write16(0xFD00, 0x0000);
    CheckInvalidInstruction(checks, executor, Mnemonic::kJnbs, 4,
                            std::make_unique<BitOperand>(0xFD00, 0),
                            std::make_unique<RelativeOperand>(128));
    checks.Equal("source word", ReadWord(raw, 0xFD00), 0x0000);
    checks.Equal("PSW", machine.ReadRegister(kPSW), kInitialPsw);
  }

  {
    checks.BeginCase("JNBS rejects an unencodable bit address");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kJnbs, 4,
                            std::make_unique<BitOperand>(0xFD01, 3),
                            std::make_unique<RelativeOperand>(0));
  }
}

}  // namespace c166::test::jnbs
