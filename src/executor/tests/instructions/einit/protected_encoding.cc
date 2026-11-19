#include "instructions/instruction_test.h"

namespace c166::test::einit {

// C166S V1 SubSystem User's Manual v1.6, pp. 3-16, 6-38, 8-23 and 9-2.
void TestProtectedEncoding(Checks& checks) {
  using enum RegisterId;
  constexpr PhysicalAddress kSysconAddress = 0xFF12;

  {
    checks.BeginCase("EINIT releases RSTOUT and locks SYSCON and DISWDT until reset");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.WriteRegister(kSYSCON, 0x8005);
    checks.Equal("RSTOUT after reset", machine.ResetOutputHigh(), false);

    ExecuteInstruction(checks, executor, Mnemonic::kEinit, 4);
    CheckCompleted(checks, machine, 4, kInitialPsw);
    checks.Equal("RSTOUT after EINIT", machine.ResetOutputHigh(), true);

    machine.WriteRegister(kSYSCON, 0);
    machine.Write8(kSysconAddress, 0xFF);
    machine.Write8(kSysconAddress + 1, 0xFF);
    checks.Equal("locked SYSCON", machine.ReadRegister(kSYSCON), 0x8005);
    ExecuteInstruction(checks, executor, Mnemonic::kDiswdt, 4);
    checks.Equal("watchdog reset remains enabled", machine.WatchdogResetEnabled(), true);

    machine.Reset();
    checks.Equal("RSTOUT after reset", machine.ResetOutputHigh(), false);
    checks.Equal("SYSCON after reset", machine.ReadRegister(kSYSCON), 0);
    machine.WriteRegister(kSYSCON, 0x0005);
    checks.Equal("SYSCON unlocked by reset", machine.ReadRegister(kSYSCON), 0x0005);
    ExecuteInstruction(checks, executor, Mnemonic::kDiswdt, 4);
    checks.Equal("DISWDT accepted after reset", machine.WatchdogResetEnabled(), false);
  }

  {
    checks.BeginCase("EINIT leaves a previously disabled watchdog stopped");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kDiswdt, 4);
    ExecuteInstruction(checks, executor, Mnemonic::kEinit, 4);

    checks.Equal("watchdog reset remains disabled", machine.WatchdogResetEnabled(), false);
    checks.Equal("watchdog counter remains stopped", machine.WatchdogCounterEnabled(), false);
    CheckCompleted(checks, machine, 8, kInitialPsw);
  }

  {
    checks.BeginCase("EINIT protection fault saves the faulting IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    Instruction malformed{Mnemonic::kEinit, std::nullopt, {}, 4};
    malformed.protection_fault = true;

    executor.Execute(malformed);
    CheckHardwareTrap(checks, machine, kClassBVector, 0x0008);
    checks.Equal("saved IP", ReadWord(raw, kResetStackPointer - 6), kStart.offset);
    checks.Equal("RSTOUT remains low", machine.ResetOutputHigh(), false);
    machine.WriteRegister(kSYSCON, 0x0005);
    checks.Equal("SYSCON remains writable", machine.ReadRegister(kSYSCON), 0x0005);
  }
}

}  // namespace c166::test::einit
