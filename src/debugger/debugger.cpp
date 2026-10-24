#include "debugger/debugger.h"

namespace c166 {
Debugger::Debugger(ICPUCore& core) : core_(core) {}

RunResult Debugger::Step(size_t count) { return Execute(count, std::nullopt, false); }

RunResult Debugger::Run(const RunOptions& options) {
  return Execute(options.max_instructions, options.stop_address, true);
}

RunResult Debugger::Execute(size_t limit, const std::optional<CodeAddress>& stop_address,
                            bool honor_breakpoints) {
  RunResult result;
  const auto initial = core_.Snapshot();
  if (initial.state == CoreState::kFaulted) {
    result.last_step = initial.last_fault;
    result.reason = result.last_step.has_value() && result.last_step->status == StepStatus::kTrap
                        ? StopReason::kTrap
                        : StopReason::kFault;
    return result;
  }
  for (size_t index = 0; index < limit; ++index) {
    const auto snapshot = core_.Snapshot();
    if (snapshot.state == CoreState::kFaulted) {
      result.last_step = snapshot.last_fault;
      result.reason = result.last_step.has_value() && result.last_step->status == StepStatus::kTrap
                          ? StopReason::kTrap
                          : StopReason::kFault;
      return result;
    }
    const auto pc = snapshot.cpu.pc;
    if (stop_address.has_value() && pc == *stop_address) {
      result.reason = StopReason::kStopAddress;
      return result;
    }
    if (honor_breakpoints && breakpoints_.contains(ToPhysicalAddress(pc))) {
      result.reason = StopReason::kBreakpoint;
      return result;
    }

    result.last_step = core_.Tick();
    if (trace_ != nullptr) {
      trace_->OnStep(*result.last_step, core_.Snapshot());
    }
    if (result.last_step->status != StepStatus::kCompleted) {
      result.reason =
          result.last_step->status == StepStatus::kTrap ? StopReason::kTrap : StopReason::kFault;
      return result;
    }
    ++result.completed_instructions;
  }
  const auto pc = core_.Snapshot().cpu.pc;
  if (stop_address.has_value() && pc == *stop_address) {
    result.reason = StopReason::kStopAddress;
  } else if (honor_breakpoints && breakpoints_.contains(ToPhysicalAddress(pc))) {
    result.reason = StopReason::kBreakpoint;
  } else {
    result.reason = StopReason::kStepLimit;
  }
  return result;
}

void Debugger::AddBreakpoint(CodeAddress address) {
  breakpoints_.insert(ToPhysicalAddress(address));
}

void Debugger::RemoveBreakpoint(CodeAddress address) {
  breakpoints_.erase(ToPhysicalAddress(address));
}

void Debugger::SetTraceSink(ITraceSink* sink) { trace_ = sink; }

CoreSnapshot Debugger::Snapshot() const { return core_.Snapshot(); }
}  // namespace c166
