#pragma once

#include <memory>
#include <optional>
#include <utility>

#include "c166s_v1_constants.h"
#include "executor/executor.h"
#include "helper.h"
#include "memory/machine.h"
#include "memory/raw_memory.h"
#include "operands.h"

namespace c166::test::mov {

inline constexpr CodeAddress kStart{0, 0x0200};
inline constexpr Word kPreservedPsw = 0x2040 | kCarry | kOverflow;
inline constexpr Word kInitialPsw = kPreservedPsw | kNegative | kZero | kEndOfTable;

inline void Prepare(Machine& machine) {
  machine.SetPC(kStart);
  machine.WriteRegister(RegisterId::kPSW, kInitialPsw);
}

inline void ExecuteMov(Executor& executor, Byte size, std::unique_ptr<Operand> destination,
                       std::unique_ptr<Operand> source) {
  Instruction instruction{Mnemonic::kMov, std::nullopt, {}, size};
  instruction.operands.push_back(std::move(destination));
  instruction.operands.push_back(std::move(source));
  executor.Execute(instruction);
}

// Word memory is little-endian (p. 4-3). Inspect raw bytes independently of Machine::Read16.
inline Word ReadWord(const RawMemory& raw, PhysicalAddress address) {
  return static_cast<Word>(raw.Read8(address) | (raw.Read8(address + 1) << 8));
}

// MOV encodings and flags are listed on pp. 6-58 and 6-59.
inline void CheckCompleted(Checks& checks, const Machine& machine, Byte size, Word expected_psw) {
  checks.Equal("CSP", machine.GetPC().segment, kStart.segment);
  checks.Equal("IP", machine.GetPC().offset, kStart.offset + size);
  checks.Equal("PSW", machine.ReadRegister(RegisterId::kPSW), expected_psw);
  checks.Equal("TFR", machine.ReadRegister(RegisterId::kTFR), 0);
}

// Class B saves the next IP for ILLOPA (pp. 3-24, 3-26–3-31).
inline void CheckTrap(Checks& checks, const Machine& machine, const RawMemory& raw, Byte size,
                      Word flag, bool segmented = true) {
  const Word frame = kResetStackPointer - (segmented ? 6 : 4);
  checks.Equal("TFR", machine.ReadRegister(RegisterId::kTFR), flag);
  checks.Equal("CSP", machine.GetPC().segment, kClassBVector.segment);
  checks.Equal("IP", machine.GetPC().offset, kClassBVector.offset);
  checks.Equal("PSW.ILVL", machine.ReadRegister(RegisterId::kPSW) & kPriorityMask, kPriorityMask);
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

}  // namespace c166::test::mov
