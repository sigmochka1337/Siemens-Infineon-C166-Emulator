#include "instructions/instruction_test.h"

namespace c166::test::ashr {

void TestRegisterCount(Checks& checks) {
  using enum RegisterId;

  const struct {
    Word value;
    Word count;
    Word result;
    Word flags;
  } cases[] = {
      {0x8001, 0xABF0, 0x8001, kNegative},
      {0x8003, 0xABF1, 0xC001, kCarry | kNegative},
      {0x8001, 0xABF2, 0xE000, kOverflow | kNegative},
      {0x0003, 0xABF2, 0x0000, kCarry | kOverflow | kZero},
      {0xC000, 0xABFF, 0xFFFF, kCarry | kNegative},
  };

  for (const auto& test : cases) {
    const auto name =
        std::format("ASHR 0x{:04X} with register count 0x{:X}", test.value, test.count);
    checks.BeginCase(name);
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, test.value);
    machine.WriteRegister(kR2, test.count);
    ExecuteInstruction(checks, executor, Mnemonic::kAshr, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR2));
    checks.Equal("R15", machine.ReadRegister(kR15), test.result);
    checks.Equal("R2", machine.ReadRegister(kR2), test.count);
    CheckCompleted(checks, machine, 2, kControlPsw | test.flags);
  }

  {
    checks.BeginCase("ASHR R15, R15 reads the original count");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 3);
    ExecuteInstruction(checks, executor, Mnemonic::kAshr, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR15));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x0000);
    CheckCompleted(checks, machine, 2, kControlPsw | kZero | kOverflow);
  }

  {
    checks.BeginCase("ASHR count register must be a word GPR");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kAshr, 2,
                            std::make_unique<RegisterOperand>(kR15),
                            std::make_unique<RegisterOperand>(kMDH));
  }
}

}  // namespace c166::test::ashr
