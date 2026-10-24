#pragma once

#include <span>

#include "common/execution.h"

namespace c166 {
class ICPUCore {
 public:
  virtual StepResult Tick() = 0;
  virtual void Reset() = 0;
  virtual Result<void> SetEntryPoint(CodeAddress address) = 0;
  virtual Result<void> LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) = 0;
  virtual Result<void> WriteRegister(RegisterId id, Word value) = 0;
  virtual Result<Byte> PeekMemory(PhysicalAddress address) const = 0;
  virtual Result<Instruction> InspectInstruction(CodeAddress address) const = 0;
  virtual CoreSnapshot Snapshot() const = 0;
  virtual ~ICPUCore() = default;
};
}  // namespace c166
