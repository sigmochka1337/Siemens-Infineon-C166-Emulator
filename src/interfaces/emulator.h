#pragma once

#include <cstddef>
#include <optional>

#include "interfaces/cpu_core.h"

namespace c166 {
enum class StopReason : uint8_t { kStepLimit, kBreakpoint, kStopAddress, kFault, kTrap };
struct RunOptions {
  size_t max_instructions = 100000;
  std::optional<CodeAddress> stop_address;
};
struct RunResult {
  StopReason reason = StopReason::kStepLimit;
  size_t completed_instructions = 0;
  std::optional<StepResult> last_step;
};
class ITraceSink {
 public:
  virtual void OnStep(const StepResult& result, const CoreSnapshot& after) = 0;
  virtual ~ITraceSink() = default;
};
class IEmulator {
 public:
  // Step ignores breakpoints, allowing a breakpoint to be stepped over.
  virtual RunResult Step(size_t count = 1) = 0;
  // Breakpoints and stop_address are checked BEFORE executing an instruction.
  virtual RunResult Run(const RunOptions& options = {}) = 0;
  virtual void AddBreakpoint(CodeAddress address) = 0;
  virtual void RemoveBreakpoint(CodeAddress address) = 0;
  virtual void SetTraceSink(ITraceSink* sink) = 0;  // Non-owning; nullptr disables trace.
  virtual CoreSnapshot Snapshot() const = 0;
  virtual ~IEmulator() = default;
};
}  // namespace c166
