#include "instructions/instruction_test.h"

namespace c166::test::or_word {

void TestImmediate3ToGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("OR R15, #7");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 0xC531);
    ExecuteInstruction(checks, executor, Mnemonic::kOr, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kR15),
                       std::make_unique<ImmediateOperand>(7));
    checks.Equal("R15", machine.ReadRegister(kR15), 0xC537);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("OR compact immediate must fit in three bits");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR15, 0xC531);
    CheckInvalidInstruction(checks, executor, Mnemonic::kOr, 2,
                            std::make_unique<RegisterOperand>(kR15),
                            std::make_unique<ImmediateOperand>(8));
    checks.Equal("R15", machine.ReadRegister(kR15), 0xC531);
  }

  {
    checks.BeginCase("OR compact immediate requires a GPR");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kOr, 2,
                            std::make_unique<RegisterOperand>(kMDH),
                            std::make_unique<ImmediateOperand>(0));
  }
}

}  // namespace c166::test::or_word
