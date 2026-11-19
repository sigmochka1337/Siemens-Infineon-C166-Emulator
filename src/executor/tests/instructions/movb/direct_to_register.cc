#include "instructions/move_test.h"

namespace c166::test::movb {

void TestDirectToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("F3 MOVB MDH, mem");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kMDH, 0xA55A);
    machine.Write16(0xFA80, 0x427F);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<RegisterOperand>(kMDH),
                std::make_unique<DirectMemoryOperand>(0xFA81));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x0042);
    CheckCompleted(checks, machine, 4, kPreservedPsw);
  }

  {
    checks.BeginCase("F3 MOVB RH0, mem via DPP1");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xA55A);
    machine.WriteRegister(kDPP1, 0x40);
    machine.Write16(0x100000, 0x807F);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<RegisterOperand>(kRH0),
                std::make_unique<DirectMemoryOperand>(0x4001));

    checks.Equal("R0", machine.ReadRegister(kR0), 0x805A);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative | kEndOfTable);
  }
}

}  // namespace c166::test::movb
