#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "src/includes/cpu_state.h"
#include "src/includes/executor.h"

namespace c166 {

struct MemoryView {
  std::uint32_t address = 0;
  std::vector<std::uint8_t> bytes;
};

struct DebugSnapshot {
  CpuState cpu{};
  std::optional<Instruction> current_instruction;
  StopReason reason = StopReason::kCompleted;
};

class IDebugger {
 public:
  virtual ~IDebugger() = default;
  virtual void AddBreakpoint(std::uint32_t address) = 0;
  virtual void RemoveBreakpoint(std::uint32_t address) = 0;
  virtual void ClearBreakpoints() = 0;
  virtual StepResult Step(std::size_t count = 1) = 0;
  virtual StepResult StepInto() = 0;
  virtual StepResult StepOut() = 0;
  virtual StepResult Run() = 0;
  virtual DebugSnapshot Snapshot() const = 0;
  virtual MemoryView ReadMemory(std::uint32_t address,
                                std::size_t size) const = 0;
  virtual void WriteMemory(std::uint32_t address,
                           const std::vector<std::uint8_t>& bytes) = 0;
  virtual void WriteRegister(std::uint8_t index, std::uint16_t value) = 0;
};

}  // namespace c166
