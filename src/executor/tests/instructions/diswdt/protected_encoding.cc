#include "instructions/instruction_test.h"

namespace c166::test::diswdt {

void TestProtectedEncoding(Checks& checks) {
  // C166S V1 SubSystem User's Manual v1.6, pp. 6-33, 9-2–9-3.
  constexpr PhysicalAddress kWatchdog = 0xFEAE;
  constexpr PhysicalAddress kWatchdogControl = 0xFFAE;
  constexpr Word kTimerEnable = 0x0040;

  {
    checks.BeginCase("DISWDT disables the watchdog until reset");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);

    ExecuteInstruction(checks, executor, Mnemonic::kDiswdt, 4);
    CheckCompleted(checks, machine, 4, kInitialPsw);
    checks.Equal("watchdog resets enabled", machine.WatchdogResetEnabled(), false);
    checks.Equal("counter enabled", machine.WatchdogCounterEnabled(), false);

    machine.Write16(kWatchdogControl, 0x5A81);
    ExecuteInstruction(checks, executor, Mnemonic::kSrvwdt, 4);
    checks.Equal("SRVWDT reloads the stopped counter", machine.Read16(kWatchdog), 0x5A00);
    checks.Equal("counter enabled after reload", machine.WatchdogCounterEnabled(), false);

    machine.Reset();
    checks.Equal("watchdog resets enabled after reset", machine.WatchdogResetEnabled(), true);
    checks.Equal("counter enabled after reset", machine.WatchdogCounterEnabled(), true);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kDiswdt, 4);
    checks.Equal("reset clears the lock", machine.WatchdogResetEnabled(), false);
  }

  {
    checks.BeginCase("DISWDT suppresses watchdog resets in interrupt mode");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    machine.Write16(kWatchdogControl, 0x0081 | kTimerEnable);

    ExecuteInstruction(checks, executor, Mnemonic::kDiswdt, 4);
    CheckCompleted(checks, machine, 4, kInitialPsw);
    checks.Equal("watchdog resets enabled", machine.WatchdogResetEnabled(), false);
    checks.Equal("TIMEN keeps the counter enabled", machine.WatchdogCounterEnabled(), true);
  }

  {
    checks.BeginCase("DISWDT has no effect after SRVWDT");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    ExecuteInstruction(checks, executor, Mnemonic::kSrvwdt, 4);

    ExecuteInstruction(checks, executor, Mnemonic::kDiswdt, 4);
    CheckCompleted(checks, machine, 8, kInitialPsw);
    checks.Equal("watchdog resets enabled", machine.WatchdogResetEnabled(), true);
  }

  {
    checks.BeginCase("DISWDT protection fault saves the faulting IP");
    RawMemory raw;
    Machine machine(raw);
    Executor executor(machine);
    Prepare(machine);
    Instruction malformed{Mnemonic::kDiswdt, std::nullopt, {}, 4};
    malformed.protection_fault = true;

    executor.Execute(malformed);
    CheckHardwareTrap(checks, machine, kClassBVector, 0x0008);
    checks.Equal("saved IP", ReadWord(raw, kResetStackPointer - 6), kStart.offset);
    checks.Equal("watchdog resets enabled", machine.WatchdogResetEnabled(), true);
  }
}

}  // namespace c166::test::diswdt
