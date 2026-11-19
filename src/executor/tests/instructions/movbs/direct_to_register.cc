#include "instructions/move_test.h"

namespace c166::test::movbs {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("D2 MOVBS MDH, mem at an odd byte address");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xA55A);
    machine.Write16(0xFA80, 0x807F);

    ExecuteMove(executor, Mnemonic::kMovBS, 4, std::make_unique<RegisterOperand>(kMDH),
                std::make_unique<DirectMemoryOperand>(0xFA81));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0xFF80);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }

  {
    checks.BeginCase("D2 MOVBS R0, mem via DPP0");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xA55A);
    machine.WriteRegister(kDPP0, 3);
    machine.Write16(0xFA80, 0x427F);

    ExecuteMove(executor, Mnemonic::kMovBS, 4, std::make_unique<RegisterOperand>(kR0),
                std::make_unique<DirectMemoryOperand>(0x3A81));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x0042);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }
}

}  // namespace c166::test::movbs
