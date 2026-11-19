#include "instructions/instruction_test.h"

namespace c166::test::orb {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("ORB MDH, mem via DPP1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 5);
    machine.WriteRegister(kMDH, 0x73C5);
    machine.Write16(0x14030, 0xA36A);
    ExecuteInstruction(checks, executor, Mnemonic::kOrB, 4, std::nullopt,
                       std::make_unique<RegisterOperand>(kMDH),
                       std::make_unique<DirectMemoryOperand>(0x4031));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x00E7);
    checks.Equal("addressed word", ReadWord(raw, 0x14030), 0xA36A);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }
}

}  // namespace c166::test::orb
