#pragma once

#include <exception>
#include <memory>
#include <optional>
#include <utility>

#include "c166s_v1_constants.h"
#include "executor/executor.h"
#include "helper.h"
#include "memory/machine.h"
#include "memory/raw_memory.h"
#include "operands.h"

namespace c166::test {

inline constexpr CodeAddress kStart{0, 0x0200};
inline constexpr Word kControlPsw = 0x2040;
inline constexpr Word kPreservedPsw = kControlPsw | kCarry | kOverflow;
inline constexpr Word kInitialPsw = kPreservedPsw | kNegative | kZero | kEndOfTable;

inline void Prepare(Machine& machine, Word psw = kInitialPsw, CodeAddress start = kStart) {
  machine.SetPC(start);
  machine.WriteRegister(RegisterId::kPSW, psw);
}

inline void ExecuteInstruction(Checks& checks, Executor& executor, Mnemonic mnemonic, Byte size,
                               std::optional<ConditionCode> condition = std::nullopt,
                               std::unique_ptr<Operand> first = {},
                               std::unique_ptr<Operand> second = {}) {
  Instruction instruction{mnemonic, condition, {}, size};
  if (first) {
    instruction.operands.push_back(std::move(first));
  }
  if (second) {
    instruction.operands.push_back(std::move(second));
  }
  try {
    executor.Execute(instruction);
  } catch (const std::exception& error) {
    checks.Fail(error.what());
  }
}

// Word memory is little-endian (p. 4-3). Inspect raw bytes independently of Machine::Read16.
inline Word ReadWord(const RawMemory& raw, PhysicalAddress address) {
  return static_cast<Word>(raw.Read8(address) | (raw.Read8(address + 1) << 8));
}

inline void CheckPc(Checks& checks, const Machine& machine, CodeAddress expected) {
  checks.Equal("CSP", machine.GetPC().segment, expected.segment);
  checks.Equal("IP", machine.GetPC().offset, expected.offset);
}

inline void CheckCompleted(Checks& checks, const Machine& machine, CodeAddress expected_pc,
                           Word expected_psw) {
  CheckPc(checks, machine, expected_pc);
  checks.Equal("PSW", machine.ReadRegister(RegisterId::kPSW), expected_psw);
  checks.Equal("TFR", machine.ReadRegister(RegisterId::kTFR), 0);
}

inline void CheckCompleted(Checks& checks, const Machine& machine, Byte size, Word expected_psw) {
  CheckCompleted(checks, machine, {kStart.segment, static_cast<Word>(kStart.offset + size)},
                 expected_psw);
}

inline void CheckHardwareTrap(Checks& checks, const Machine& machine, CodeAddress vector,
                              Word flag) {
  checks.Equal("TFR", machine.ReadRegister(RegisterId::kTFR), flag);
  CheckPc(checks, machine, vector);
  checks.Equal("PSW.ILVL", machine.ReadRegister(RegisterId::kPSW) & kPriorityMask, kPriorityMask);
}

// Class B saves the next IP for ILLOPA (pp. 3-24, 3-26–3-31).
inline void CheckTrap(Checks& checks, const Machine& machine, const RawMemory& raw, Byte size,
                      Word flag, bool segmented = true) {
  CheckHardwareTrap(checks, machine, kClassBVector, flag);
  const Word frame = kResetStackPointer - (segmented ? 6 : 4);
  checks.Equal("SP", machine.ReadRegister(RegisterId::kSP), frame);
  if (machine.ReadRegister(RegisterId::kSP) != frame) {
    return;
  }
  checks.Equal("saved IP", ReadWord(raw, frame), kStart.offset + size);
  if (segmented) {
    checks.Equal("saved CSP", ReadWord(raw, frame + 2), kStart.segment);
  }
  checks.Equal("saved PSW priority", ReadWord(raw, kResetStackPointer - 2) & kPriorityMask,
               kInitialPsw & kPriorityMask);
}

}  // namespace c166::test
