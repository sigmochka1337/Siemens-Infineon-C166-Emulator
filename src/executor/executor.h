#pragma once

#include "interfaces/executor.h"

namespace c166 {
// Minimal functional executor. See docs/isa_scope.md for supported forms.
class Executor final : public IExecutor {
 public:
  ExecutionResult Execute(const Instruction& instruction, IMachine& machine) const override;
};
}  // namespace c166
