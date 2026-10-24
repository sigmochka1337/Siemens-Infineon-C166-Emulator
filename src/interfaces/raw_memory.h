#pragma once

#include "common/result.h"

namespace c166 {
// Optional storage backend owned by Memory. CPU clients use IMemoryBus.
class IRawMemory {
 public:
  virtual Result<Byte> Read8(PhysicalAddress address) const = 0;
  virtual Result<void> Write8(PhysicalAddress address, Byte value) = 0;
  virtual ~IRawMemory() = default;
};
}  // namespace c166
