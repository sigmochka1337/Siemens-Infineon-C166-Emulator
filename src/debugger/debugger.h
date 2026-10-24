#pragma once

#include <set>

#include "interfaces/emulator.h"

namespace c166 {
class Debugger final : public IEmulator {
 public:
  explicit Debugger(ICPUCore& core);
  RunResult Step(size_t count = 1) override;
  RunResult Run(const RunOptions& options = {}) override;
  void AddBreakpoint(CodeAddress address) override;
  void RemoveBreakpoint(CodeAddress address) override;
  void SetTraceSink(ITraceSink* sink) override;
  CoreSnapshot Snapshot() const override;

 private:
  RunResult Execute(size_t limit, const std::optional<CodeAddress>& stop_address,
                    bool honor_breakpoints);
  ICPUCore& core_;
  std::set<PhysicalAddress> breakpoints_;
  ITraceSink* trace_ = nullptr;
};
}  // namespace c166
