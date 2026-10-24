#pragma once

#include "interfaces/decoder.h"
#include "interfaces/memory.h"

namespace c166 {
class IFetcher {
 public:
  // Read the first word, ask decoder for length, read only required bytes.
  // Never changes PC, DPP, extension state, or code segment.
  virtual Result<RawInstruction> Fetch(CodeAddress pc, IMemoryBus& memory,
                                       const IDecoder& decoder) const = 0;
  virtual ~IFetcher() = default;
};
}  // namespace c166
