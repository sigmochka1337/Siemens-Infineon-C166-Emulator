#include "instructions/move_test.h"

namespace c166::test::movb {

void TestImmediate8ToRegister(Checks& checks) {
  using enum RegisterId;

  {
    checks.BeginCase("E7 MOVB MDH, #0x80");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine, kPreservedPsw);
    machine.WriteRegister(kMDH, 0xA55A);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<RegisterOperand>(kMDH),
                std::make_unique<ImmediateOperand>(0x80));

    checks.Equal("MDH", machine.ReadRegister(kMDH), 0x0080);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative | kEndOfTable);
  }

  {
    checks.BeginCase("E7 MOVB RH0, #0xFF");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kR0, 0xA55A);

    ExecuteMove(executor, Mnemonic::kMovB, 4, std::make_unique<RegisterOperand>(kRH0),
                std::make_unique<ImmediateOperand>(0xFF));

    checks.Equal("R0", machine.ReadRegister(kR0), 0xFF5A);
    CheckCompleted(checks, machine, 4, kPreservedPsw | kNegative);
  }
}

}  // namespace c166::test::movb
