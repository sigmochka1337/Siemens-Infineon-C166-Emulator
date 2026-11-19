#include "instructions/instruction_test.h"

namespace c166::test::shl {

void TestRegisterCount(Checks& checks) {
  using enum RegisterId;

  const struct {
    Word value;
    Word count;
    Word result;
    Word flags;
  } cases[] = {
      {0x8001, 0xABF0, 0x8001, kNegative},
      {0x8003, 0xABF1, 0x0006, kCarry},
      {0xC001, 0xABF2, 0x0004, kCarry},
      {0x4000, 0xABF2, 0x0000, kCarry | kZero},
      {0x0003, 0xABFF, 0x8000, kCarry | kNegative},
  };

  for (const auto& test : cases) {
    const auto name =
        std::format("SHL 0x{:04X} with register count 0x{:X}", test.value, test.count);
    checks.BeginCase(name);
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, test.value);
    machine.WriteRegister(kR2, test.count);
    ExecuteInstruction(checks, executor, Mnemonic::kShl, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR2));
    checks.Equal("R15", machine.ReadRegister(kR15), test.result);
    checks.Equal("R2", machine.ReadRegister(kR2), test.count);
    CheckCompleted(checks, machine, 2, kControlPsw | test.flags);
  }

  {
    checks.BeginCase("SHL R15, R15 reads the original count");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 3);
    ExecuteInstruction(checks, executor, Mnemonic::kShl, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<RegisterOperand>(kR15));
    checks.Equal("R15", machine.ReadRegister(kR15), 0x0018);
    CheckCompleted(checks, machine, 2, kControlPsw);
  }

  {
    checks.BeginCase("SHL count register must be a word GPR");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kShl, 2,
                            std::make_unique<RegisterOperand>(kR15),
                            std::make_unique<RegisterOperand>(kMDH));
  }
}

}  // namespace c166::test::shl
