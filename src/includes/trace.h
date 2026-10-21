#pragma once

#include <cstddef>
#include <vector>

#include "src/includes/debugger.h"

namespace c166 {

struct TraceEntry {
  std::size_t sequence = 0;
  StepResult result{};
  CpuState state_after{};
};

class ITraceSink {
 public:
  virtual ~ITraceSink() = default;
  virtual void Append(const TraceEntry& entry) = 0;
  virtual void Flush() = 0;
};

}  // namespace c166
