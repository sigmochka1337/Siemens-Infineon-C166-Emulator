#include "instructions/instruction_test.h"

namespace c166::test::extr {

void TestInstructionRange(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("EXTR redirects short register and bit operands for exactly two instructions");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  ExecuteInstruction(checks, executor, Mnemonic::kExtr, 2, std::nullopt,
                     std::make_unique<ImmediateOperand>(2));
  ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                     std::make_unique<RegisterOperand>(kMDH),
                     std::make_unique<ImmediateOperand>(0xCAFE));
  checks.Equal("extended register", machine.Read16(0xF00C), 0xCAFE);
  checks.Equal("standard MDH", machine.ReadRegister(kMDH), 0);
  ExecuteInstruction(checks, executor, Mnemonic::kBSet, 2, std::nullopt,
                     std::make_unique<BitOperand>(0xFF10, 7));
  checks.Equal("extended bit", machine.Read16(0xF110), 0x0080);
  checks.Equal("implicit PSW", machine.ReadRegister(kPSW), kControlPsw | kZero);
  ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                     std::make_unique<RegisterOperand>(kMDH),
                     std::make_unique<ImmediateOperand>(0x1234));
  checks.Equal("standard MDH restored", machine.ReadRegister(kMDH), 0x1234);
  CheckCompleted(checks, machine, 12, kControlPsw);
}

}  // namespace c166::test::extr
