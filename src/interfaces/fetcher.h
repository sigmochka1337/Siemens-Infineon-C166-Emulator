#pragma once
#include "types.h"

namespace c166 {
class IFetcher {
 public:
  virtual ~IFetcher() = default;

  virtual Instruction Fetch(CodeAddress pc) = 0;
};
}  // namespace c166
