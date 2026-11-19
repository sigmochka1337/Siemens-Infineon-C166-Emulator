#include "instructions/instruction_test.h"

namespace c166::test::orb {

void TestRegisterToDirect(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("ORB mem, MDH via DPP1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kDPP1, 5);
    machine.WriteRegister(kMDH, 0x73C5);
    machine.Write16(0x14030, 0xA36A);
    ExecuteInstruction(checks, executor, Mnemonic::kOrB, 4, std::nullopt,
                       std::make_unique<DirectMemoryOperand>(0x4031),
                       std::make_unique<RegisterOperand>(kMDH));
    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x73C5);
    checks.Equal("addressed word", ReadWord(raw, 0x14030), 0xE76A);
    CheckCompleted(checks, machine, 4, kControlPsw | kNegative);
  }
}

}  // namespace c166::test::orb
