#pragma once
#include "types.h"
namespace c166 {
class IRawMemory {
 public:
  virtual ~IRawMemory() = default;

  virtual Byte Read8(PhysicalAddress address) const = 0;
  virtual void Write8(PhysicalAddress address, Byte value) = 0;
};
}  // namespace c166
