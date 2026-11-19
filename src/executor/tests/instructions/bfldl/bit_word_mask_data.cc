#include "instructions/instruction_test.h"

namespace c166::test::bfldl {

// C166S V1 SubSystem User's Manual v1.6, p. 6-15.
void TestBitWordMaskData(Checks& checks) {
  checks.BeginCase("BFLDL modifies selected low bits and uses the full word for N and Z");
  constexpr Word kBusConfig = 0xFF0C;
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(RegisterId::kDPP3, 5);
  machine.Write16(kBusConfig, 0x80F0);

  Instruction instruction{Mnemonic::kBfldl, std::nullopt, {}, 4};
  instruction.operands.push_back(std::make_unique<BitWordOperand>(kBusConfig));
  instruction.operands.push_back(std::make_unique<ImmediateOperand>(0x3C));
  instruction.operands.push_back(std::make_unique<ImmediateOperand>(0x5A));
  executor.Execute(instruction);

  checks.Equal("BUSCON0", machine.Read16(kBusConfig), 0x80D8);
  CheckCompleted(checks, machine, 4, kControlPsw | kNegative);

  checks.BeginCase("BFLDL sets Z when the resulting word is zero");
  Prepare(machine);
  machine.Write16(kBusConfig, 0x003C);
  instruction.operands[2] = std::make_unique<ImmediateOperand>(0);
  executor.Execute(instruction);

  checks.Equal("BUSCON0", machine.Read16(kBusConfig), 0);
  CheckCompleted(checks, machine, 4, kControlPsw | kZero);

  checks.BeginCase("BFLDL on PSW preserves flags outside the selected field");
  Prepare(machine);
  instruction.operands[0] = std::make_unique<BitWordOperand>(0xFF10);
  instruction.operands[1] = std::make_unique<ImmediateOperand>(0x40);
  executor.Execute(instruction);

  CheckCompleted(checks, machine, 4, kInitialPsw & ~0x0040);
}

}  // namespace c166::test::bfldl
