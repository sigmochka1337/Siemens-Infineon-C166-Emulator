#include "instructions/instruction_test.h"

namespace c166::test::bfldh {

// C166S V1 SubSystem User's Manual v1.6, p. 6-14.
void TestBitWordMaskData(Checks& checks) {
  checks.BeginCase("BFLDH modifies the high byte of a GPR in the current context");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.WriteRegister(RegisterId::kCP, 0xFA00);
  machine.WriteRegister(RegisterId::kR6, 0x253C);

  Instruction instruction{Mnemonic::kBfldh, std::nullopt, {}, 4};
  instruction.operands.push_back(std::make_unique<BitWordOperand>(0xFFEC));
  instruction.operands.push_back(std::make_unique<ImmediateOperand>(0xF0));
  instruction.operands.push_back(std::make_unique<ImmediateOperand>(0xAB));
  executor.Execute(instruction);

  checks.Equal("R6", machine.ReadRegister(RegisterId::kR6), 0xA53C);
  CheckCompleted(checks, machine, 4, kControlPsw | kNegative);

  checks.BeginCase("BFLDH accesses the extended SFR during EXTR");
  Prepare(machine);
  machine.Write16(0xFF0C, 0x1234);
  machine.Write16(0xF10C, 0x005A);
  ExecuteInstruction(checks, executor, Mnemonic::kExtr, 2, std::nullopt,
                     std::make_unique<ImmediateOperand>(1));
  instruction.operands[0] = std::make_unique<BitWordOperand>(0xFF0C);
  executor.Execute(instruction);

  checks.Equal("extended word", machine.Read16(0xF10C), 0xA05A);
  checks.Equal("standard word", machine.Read16(0xFF0C), 0x1234);
  CheckCompleted(checks, machine, 6, kControlPsw | kNegative);
}

}  // namespace c166::test::bfldh
