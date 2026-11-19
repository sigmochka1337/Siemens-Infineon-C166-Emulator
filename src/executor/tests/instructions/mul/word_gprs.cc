#include "instructions/instruction_test.h"

namespace c166::test::mul {

void TestWordGprs(Checks& checks) {
  using enum RegisterId;

  struct Case {
    Word left, right;
    Dword product;
    Word flags;
  };
  constexpr Case cases[] = {{0xFFFF, 2, 0xFFFFFFFE, kNegative},
                            {0x8000, 0xFFFF, 0x8000, kOverflow},
                            {0x8000, 1, 0xFFFF8000, kNegative},
                            {0, 5, 0, kZero}};
  for (const auto& [left, right, product, flags] : cases) {
    checks.BeginCase("MUL R5, R6");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR5, left);
    machine.WriteRegister(kR6, right);
    ExecuteInstruction(checks, executor, Mnemonic::kMul, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR5),
                       std::make_unique<RegisterOperand>(kR6));
    checks.Equal("MDC.MDRIU", machine.ReadRegister(kMDC) & 0x10, 0x10);
    checks.Equal("MDH", machine.ReadRegister(kMDH), product >> 16);
    checks.Equal("MDL", machine.ReadRegister(kMDL), product & 0xFFFF);
    checks.Equal("R5", machine.ReadRegister(kR5), left);
    checks.Equal("R6", machine.ReadRegister(kR6), right);
    CheckCompleted(checks, machine, 2, kControlPsw | flags);
  }
}

}  // namespace c166::test::mul
