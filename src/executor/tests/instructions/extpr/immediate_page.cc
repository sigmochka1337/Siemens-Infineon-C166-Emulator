#include "instructions/instruction_test.h"

namespace c166::test::extpr {

void TestImmediatePage(Checks& checks) {
  using enum RegisterId;

  checks.BeginCase("EXTPR immediate selects data addresses for two instructions");
  RawMemory raw;
  Machine machine(raw);
  Executor executor(machine);
  Prepare(machine);
  machine.Write16(0xC020, 0x3456);
  machine.Write16(0x414020, 0x1234);
  ExecuteInstruction(checks, executor, Mnemonic::kExtpr, 4, std::nullopt,
                     std::make_unique<ImmediateOperand>(0x0105),
                     std::make_unique<ImmediateOperand>(2));
  CheckCompleted(checks, machine, 4, kInitialPsw);
  ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                     std::make_unique<RegisterOperand>(kMDH),
                     std::make_unique<DirectMemoryOperand>(0xC020));
  checks.Equal("short SFR destination", machine.Read16(0xF00C), 0x1234);
  machine.WriteRegister(kR3, 0xC020);
  ExecuteInstruction(checks, executor, Mnemonic::kMov, 2, std::nullopt,
                     std::make_unique<RegisterOperand>(kR4),
                     IndirectMemoryOperand::CreatePlain(kR3));
  checks.Equal("indirect load", machine.ReadRegister(kR4), 0x1234);
  ExecuteInstruction(checks, executor, Mnemonic::kMov, 4, std::nullopt,
                     std::make_unique<RegisterOperand>(kMDH),
                     std::make_unique<DirectMemoryOperand>(0xC020));
  checks.Equal("standard SFR and page restored", machine.ReadRegister(kMDH), 0x3456);
  CheckCompleted(checks, machine, 14, kPreservedPsw);
}

}  // namespace c166::test::extpr
