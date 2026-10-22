#pragma once
#include "interfaces/cpu_core.h"

namespace c166 {
class IEmulator {
 public:
  virtual void Step(uint32_t cnt = 1) = 0;
  virtual void Run() = 0;
  virtual void AddBreakpoint(CodeAddress address) = 0;
  virtual void RemoveBreakpoint(CodeAddress address) = 0;

  virtual CPUSnapshot Snapshot() const = 0;

  virtual ~IEmulator() = default;
};
}  // namespace c166
