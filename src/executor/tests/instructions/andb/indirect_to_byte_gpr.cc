#include "instructions/instruction_test.h"

namespace c166::test::andb {

void TestIndirectToByteGpr(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("ANDB RH7, [R3] via DPP1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 5);
    machine.WriteRegister(kR3, 0x4031);
    machine.WriteRegister(kR7, 0xC55A);
    machine.Write16(0x14030, 0xA36A);
    ExecuteInstruction(checks, executor, Mnemonic::kAndB, 2, std::nullopt,
                       std::make_unique<RegisterOperand>(kRH7),
                       IndirectMemoryOperand::CreatePlain(kR3));
    checks.Equal("R7", machine.ReadRegister(kR7), 0x815A);
    checks.Equal("R3", machine.ReadRegister(kR3), 0x4031);
    checks.Equal("source word", ReadWord(raw, 0x14030), 0xA36A);
    CheckCompleted(checks, machine, 2, kControlPsw | kNegative);
  }

  {
    checks.BeginCase("ANDB indirect encoding only selects R0 through R3");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    CheckInvalidInstruction(checks, executor, Mnemonic::kAndB, 2,
                            std::make_unique<RegisterOperand>(kRH7),
                            IndirectMemoryOperand::CreatePlain(kR4));
  }
}

}  // namespace c166::test::andb
