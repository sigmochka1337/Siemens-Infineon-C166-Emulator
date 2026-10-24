#pragma once

#include "common/execution.h"
#include "interfaces/memory.h"

namespace c166 {
class IExecutor {
 public:
  // Owns semantics, PC advancement, flags, stack, extension countdown and flow
  // events. Unsupported forms must fail without any machine mutation.
  virtual ExecutionResult Execute(const Instruction& instruction, IMachine& machine) const = 0;
  virtual ~IExecutor() = default;
};
}  // namespace c166
