#include "instructions/instruction_test.h"

namespace c166::test::shr {

void TestImmediateCount(Checks& checks) {
  using enum RegisterId;

  const struct {
    Word value;
    Word count;
    Word result;
    Word flags;
  } cases[] = {
      {0x8001, 0, 0x8001, kNegative},       {0x8003, 1, 0x4001, kCarry},
      {0x8001, 2, 0x2000, kOverflow},       {0x0003, 2, 0x0000, kCarry | kOverflow | kZero},
      {0x4000, 15, 0x0000, kCarry | kZero},
  };

  for (const auto& test : cases) {
    const auto name =
        std::format("SHR 0x{:04X} with immediate count 0x{:X}", test.value, test.count);
    checks.BeginCase(name);
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, test.value);
    ExecuteInstruction(checks, executor, Mnemonic::kShr, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<ImmediateOperand>(test.count));
    checks.Equal("R15", machine.ReadRegister(kR15), test.result);
    CheckCompleted(checks, machine, 2, kControlPsw | test.flags);
  }

  {
    checks.BeginCase("SHR immediate count must fit in four bits");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 3);
    CheckInvalidInstruction(checks, executor, Mnemonic::kShr, 2,
                            std::make_unique<RegisterOperand>(kR15),
                            std::make_unique<ImmediateOperand>(16));
    checks.Equal("R15", machine.ReadRegister(kR15), 3);
  }
}

}  // namespace c166::test::shr
