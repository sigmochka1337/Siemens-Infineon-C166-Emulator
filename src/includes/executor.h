#pragma once

#include <cstdint>

#include "src/includes/control_instructions.h"
#include "src/includes/cpu_state.h"
#include "src/includes/data_transfer.h"
#include "src/includes/decoder.h"

namespace c166 {

enum class StopReason : std::uint8_t {
  kCompleted,
  kBreakpoint,
  kHalted,
  kFault,
  kInvalidInstruction
};

struct StepResult {
  StopReason reason = StopReason::kCompleted;
  std::uint32_t pc_before = 0;
  std::uint32_t pc_after = 0;
  Instruction instruction{};
};

class IExecutor {
 public:
  virtual ~IExecutor() = default;
  virtual StepResult Step(CpuState& state, IMemory& memory) = 0;
  virtual StepResult Execute(const Instruction& instruction, CpuState& state,
                             IMemory& memory) = 0;
};

}  // namespace c166
