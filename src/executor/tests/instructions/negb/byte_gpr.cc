#include "instructions/instruction_test.h"

namespace c166::test::negb {

void TestByteGpr(Checks& checks) {
  using enum RegisterId;

  struct Case {
    Byte input, result;
    Word flags;
  };
  constexpr Case cases[] = {{0, 0, kZero},
                            {1, 0xFF, kCarry | kNegative},
                            {0x80, 0x80, kEndOfTable | kOverflow | kCarry | kNegative},
                            {0xFF, 1, kCarry}};
  for (const auto& [input, result, flags] : cases) {
    checks.BeginCase("NEGB RH7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR7, (input << 8) | 0x5A);
    ExecuteInstruction(checks, executor, Mnemonic::kNegB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7));
    checks.Equal("R7", machine.ReadRegister(kR7), (result << 8) | 0x5A);
    CheckCompleted(checks, machine, 2, kControlPsw | flags);
  }
}

}  // namespace c166::test::negb
