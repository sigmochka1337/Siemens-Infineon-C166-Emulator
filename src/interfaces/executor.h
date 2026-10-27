#pragma once
#include "types.h"

namespace c166 {
class IExecutor {
 public:
  virtual ~IExecutor() = default;

  virtual void Execute(Instruction instruction) = 0;
};
}  // namespace c166
