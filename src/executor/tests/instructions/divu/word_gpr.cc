#include "instructions/instruction_test.h"

namespace c166::test::divu {

void TestWordGpr(Checks& checks) {
  using enum RegisterId;

  struct Case {
    Word high, low, divisor, quotient, remainder, flags;
  };
  constexpr Case cases[] = {
      {0xFFFE, 0xFFFF, 1, 0xFFFF, 0, kNegative}, {0x1234, 10, 3, 3, 1, 0}, {0, 1, 3, 0, 1, kZero}};
  for (const auto& [high, low, divisor, quotient, remainder, flags] : cases) {
    checks.BeginCase("DIVU R3 computes quotient and remainder");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, high);
    machine.WriteRegister(kMDL, low);
    machine.WriteRegister(kR3, divisor);
    machine.WriteRegister(kMDC, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kDivU, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR3));
    checks.Equal("MDC.MDRIU", machine.ReadRegister(kMDC) & 0x10, 0x10);
    checks.Equal("MDH", machine.ReadRegister(kMDH), remainder);
    checks.Equal("MDL", machine.ReadRegister(kMDL), quotient);
    checks.Equal("R3", machine.ReadRegister(kR3), divisor);
    CheckCompleted(checks, machine, 2, kControlPsw | flags);
  }

  struct OverflowCase {
    Word high, low, divisor;
  };
  constexpr OverflowCase overflows[] = {{0, 7, 0}};
  for (const auto& [high, low, divisor] : overflows) {
    checks.BeginCase("DIVU sets V without entering a trap");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, high);
    machine.WriteRegister(kMDL, low);
    machine.WriteRegister(kR3, divisor);
    machine.WriteRegister(kMDC, 0);
    ExecuteInstruction(checks, executor, Mnemonic::kDivU, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR3));
    // N and Z are undefined when V is set.
    checks.Equal("defined PSW bits", machine.ReadRegister(kPSW) & ~(kNegative | kZero),
                 kControlPsw | kOverflow);
    checks.Equal("MDC.MDRIU", machine.ReadRegister(kMDC) & 0x10, 0x10);
    checks.Equal("TFR", machine.ReadRegister(kTFR), 0);
    CheckPc(checks, machine, {0, kStart.offset + 2});
  }
}

}  // namespace c166::test::divu
