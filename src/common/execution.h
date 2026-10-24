#pragma once

#include <optional>

#include "common/instruction.h"
#include "common/result.h"
#include "common/state.h"

namespace c166 {
enum class StepStatus : uint8_t { kCompleted, kFaulted, kTrap, kStopped };
enum class FaultStage : uint8_t { kFetch, kDecode, kExecute, kControl };
enum class FlowKind : uint8_t { kSequential, kBranch, kCall, kReturn };
struct ExecutionResult {
  StepStatus status = StepStatus::kCompleted;
  FlowKind flow = FlowKind::kSequential;
  std::optional<Diagnostic> diagnostic;
};
struct StepResult {
  StepStatus status = StepStatus::kCompleted;
  CodeAddress pc_before{};
  CodeAddress pc_after{};
  std::optional<Instruction> instruction;
  std::optional<Diagnostic> diagnostic;
  std::optional<FaultStage> fault_stage;
  FlowKind flow = FlowKind::kSequential;
};
enum class CoreState : uint8_t { kReady, kFaulted };
struct CoreSnapshot {
  CpuSnapshot cpu{};
  CoreState state = CoreState::kReady;
  std::optional<StepResult> last_fault;
};
}  // namespace c166
